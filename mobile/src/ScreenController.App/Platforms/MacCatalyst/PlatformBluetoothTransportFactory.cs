using System.Collections.Concurrent;
using System.Threading.Channels;
using CoreBluetooth;
using CoreFoundation;
using Foundation;
using ScreenController.Protocol;

namespace ScreenController.App.Services;

public sealed class PlatformBluetoothTransportFactory : IControllerTransportFactory
{
    private readonly CentralDelegate centralDelegate = new();
    private readonly CBCentralManager central;
    private readonly ConcurrentDictionary<string, CBPeripheral> peripherals = new();
    private readonly ConcurrentDictionary<string, PeripheralDelegate> peripheralDelegates = new();

    public PlatformBluetoothTransportFactory()
    {
        central = new CBCentralManager(centralDelegate, DispatchQueue.MainQueue);
        centralDelegate.Discovered += peripheral => peripherals[peripheral.Identifier.AsString()] = peripheral;
    }

    public async IAsyncEnumerable<DeviceCandidate> ScanAsync(
        [System.Runtime.CompilerServices.EnumeratorCancellation] CancellationToken cancellationToken)
    {
        await centralDelegate.WaitUntilReadyAsync(cancellationToken);
        var output = Channel.CreateUnbounded<DeviceCandidate>();
        void OnDiscovered(CBPeripheral peripheral, int rssi) =>
            output.Writer.TryWrite(new(peripheral.Identifier.AsString(), "ScreenController", rssi));
        centralDelegate.CandidateDiscovered += OnDiscovered;
        central.ScanForPeripherals([CBUUID.FromString(ProtocolConstants.ServiceUuid.ToString())]);
        using var registration = cancellationToken.Register(() => output.Writer.TryComplete());
        try { await foreach (var item in output.Reader.ReadAllAsync(cancellationToken)) yield return item; }
        finally { central.StopScan(); centralDelegate.CandidateDiscovered -= OnDiscovered; }
    }

    public async Task<IControllerTransport> ConnectAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken)
    {
        await centralDelegate.WaitUntilReadyAsync(cancellationToken);
        if (!peripherals.TryGetValue(enrollment.DeviceId, out var peripheral))
        {
            var uuid = new NSUuid(enrollment.DeviceId);
            peripheral = central.RetrievePeripheralsWithIdentifiers([uuid]).FirstOrDefault()
                ?? throw new InvalidOperationException("Scan again; macOS no longer knows this Bluetooth device.");
        }

        await centralDelegate.ConnectAsync(central, peripheral, cancellationToken);
        var peripheralDelegate = new PeripheralDelegate();
        peripheral.Delegate = peripheralDelegate;
        peripheralDelegates[peripheral.Identifier.AsString()] = peripheralDelegate;
        var channel = await peripheralDelegate.OpenChannelAsync(peripheral, (ushort)enrollment.Psm, cancellationToken);
        return new AppleL2CapTransport(channel);
    }

    private sealed class CentralDelegate : CBCentralManagerDelegate
    {
        private readonly TaskCompletionSource ready = new(TaskCreationOptions.RunContinuationsAsynchronously);
        private readonly ConcurrentDictionary<string, TaskCompletionSource> connections = new();
        public event Action<CBPeripheral>? Discovered;
        public event Action<CBPeripheral, int>? CandidateDiscovered;

        public override void UpdatedState(CBCentralManager central)
        {
            if (central.State == CBManagerState.PoweredOn) ready.TrySetResult();
            else if (central.State is CBManagerState.Unsupported or CBManagerState.Unauthorized)
                ready.TrySetException(new InvalidOperationException($"CoreBluetooth is {central.State}."));
        }

        public Task WaitUntilReadyAsync(CancellationToken token) => ready.Task.WaitAsync(token);

        public override void DiscoveredPeripheral(CBCentralManager central, CBPeripheral peripheral,
            NSDictionary advertisementData, NSNumber rssi)
        {
            Discovered?.Invoke(peripheral);
            CandidateDiscovered?.Invoke(peripheral, rssi.Int32Value);
        }

        public Task ConnectAsync(CBCentralManager central, CBPeripheral peripheral, CancellationToken token)
        {
            if (peripheral.State == CBPeripheralState.Connected) return Task.CompletedTask;
            var source = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
            connections[peripheral.Identifier.AsString()] = source;
            central.ConnectPeripheral(peripheral);
            return source.Task.WaitAsync(token);
        }

        public override void ConnectedPeripheral(CBCentralManager central, CBPeripheral peripheral)
        {
            if (connections.TryRemove(peripheral.Identifier.AsString(), out var source)) source.TrySetResult();
        }

        public override void FailedToConnectPeripheral(CBCentralManager central, CBPeripheral peripheral, NSError? error)
        {
            if (connections.TryRemove(peripheral.Identifier.AsString(), out var source))
                source.TrySetException(new IOException(error?.LocalizedDescription ?? "Connection failed."));
        }
    }

    private sealed class PeripheralDelegate : CBPeripheralDelegate
    {
        private TaskCompletionSource<CBL2CapChannel>? channelSource;
        public Task<CBL2CapChannel> OpenChannelAsync(CBPeripheral peripheral, ushort psm, CancellationToken token)
        {
            var source = new TaskCompletionSource<CBL2CapChannel>(TaskCreationOptions.RunContinuationsAsynchronously);
            channelSource = source;
            peripheral.OpenL2CapChannel(psm);
            return source.Task.WaitAsync(token);
        }

        public override void DidOpenL2CapChannel(CBPeripheral peripheral, CBL2CapChannel? channel, NSError? error)
        {
            var source = Interlocked.Exchange(ref channelSource, null);
            if (source is null) return;
            if (channel is null || error is not null) source.TrySetException(new IOException(error?.LocalizedDescription ?? "Could not open L2CAP channel."));
            else source.TrySetResult(channel);
        }
    }

    private sealed class AppleL2CapTransport : IControllerTransport
    {
        private readonly CBL2CapChannel channel;
        public AppleL2CapTransport(CBL2CapChannel channel)
        {
            this.channel = channel;
            channel.InputStream.Open();
            channel.OutputStream.Open();
        }
        public bool IsConnected => channel.InputStream.Status is NSStreamStatus.Open or NSStreamStatus.Reading;

        public async ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken token)
        {
            var temporary = new byte[buffer.Length];
            while (!channel.InputStream.HasBytesAvailable())
            {
                token.ThrowIfCancellationRequested();
                if (channel.InputStream.Error is { } error) throw new IOException(error.LocalizedDescription);
                await Task.Delay(5, token);
            }
            var count = (int)channel.InputStream.Read(temporary, (nuint)temporary.Length);
            if (count > 0) temporary.AsSpan(0, count).CopyTo(buffer.Span);
            return count;
        }

        public async ValueTask WriteAsync(ReadOnlyMemory<byte> buffer, IProgress<TransferProgress>? progress, CancellationToken token)
        {
            var bytes = buffer.ToArray();
            var sent = 0;
            while (sent < bytes.Length)
            {
                while (!channel.OutputStream.HasSpaceAvailable()) { token.ThrowIfCancellationRequested(); await Task.Delay(5, token); }
                var remaining = bytes.AsSpan(sent).ToArray();
                var count = (int)channel.OutputStream.Write(remaining, (nuint)remaining.Length);
                if (count <= 0) throw new IOException(channel.OutputStream.Error?.LocalizedDescription ?? "L2CAP write failed.");
                sent += count;
                progress?.Report(new(sent, bytes.Length));
            }
        }

        public ValueTask DisposeAsync() { channel.InputStream.Close(); channel.OutputStream.Close(); channel.Dispose(); return ValueTask.CompletedTask; }
    }
}
