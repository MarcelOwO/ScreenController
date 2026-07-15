using System.Threading.Channels;
using Android.Bluetooth;
using Android.Bluetooth.LE;
using Android.Content;
using Android.OS;
using Java.Util;
using ScreenController.Protocol;

namespace ScreenController.App.Services;

public sealed class PlatformBluetoothTransportFactory : IControllerTransportFactory
{
    private readonly BluetoothAdapter adapter;

    public PlatformBluetoothTransportFactory()
    {
        var manager = (BluetoothManager?)Android.App.Application.Context.GetSystemService(Context.BluetoothService);
        adapter = manager?.Adapter ?? throw new PlatformNotSupportedException("This Android device has no Bluetooth adapter.");
    }

    public async IAsyncEnumerable<DeviceCandidate> ScanAsync(
        [System.Runtime.CompilerServices.EnumeratorCancellation] CancellationToken cancellationToken)
    {
        await EnsurePermissionAsync();
        if (!adapter.IsEnabled) throw new InvalidOperationException("Turn Bluetooth on before scanning.");
        var scanner = adapter.BluetoothLeScanner ?? throw new InvalidOperationException("Bluetooth LE scanning is unavailable.");
        var channel = Channel.CreateUnbounded<DeviceCandidate>();
        var callback = new ScanResultsCallback(channel.Writer);
        var serviceUuid = ParcelUuid.FromString(ProtocolConstants.ServiceUuid.ToString())
            ?? throw new InvalidOperationException("Could not create the ScreenController service UUID.");
        var filter = new ScanFilter.Builder().SetServiceUuid(serviceUuid)!.Build()
            ?? throw new InvalidOperationException("Could not create the Bluetooth scan filter.");
        var settings = new ScanSettings.Builder()
            .SetScanMode(Android.Bluetooth.LE.ScanMode.LowLatency)!.Build()
            ?? throw new InvalidOperationException("Could not create the Bluetooth scan settings.");
        scanner.StartScan([filter], settings, callback);
        using var registration = cancellationToken.Register(() => channel.Writer.TryComplete());
        try
        {
            await foreach (var candidate in channel.Reader.ReadAllAsync(cancellationToken)) yield return candidate;
        }
        finally
        {
            scanner.StopScan(callback);
            callback.Dispose();
        }
    }

    public async Task<IControllerTransport> ConnectAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken)
    {
        await EnsurePermissionAsync();
        var device = adapter.GetRemoteDevice(enrollment.DeviceId)
            ?? throw new InvalidOperationException("The saved Bluetooth device is no longer available.");
        var socket = device.CreateL2capChannel(enrollment.Psm)
            ?? throw new IOException("Android could not create a secure LE L2CAP channel.");
        try
        {
            await Task.Run(socket.Connect, cancellationToken);
            return new AndroidL2CapTransport(socket);
        }
        catch
        {
            socket.Dispose();
            throw;
        }
    }

    private static async Task EnsurePermissionAsync()
    {
        var status = await Permissions.RequestAsync<Permissions.Bluetooth>();
        if (status != PermissionStatus.Granted)
            throw new UnauthorizedAccessException("Nearby Bluetooth permission is required.");
    }

    private sealed class ScanResultsCallback(ChannelWriter<DeviceCandidate> writer) : ScanCallback
    {
        public override void OnScanResult(ScanCallbackType callbackType, ScanResult? result)
        {
            var device = result?.Device;
            if (device is null) return;
            writer.TryWrite(new DeviceCandidate(device.Address!, "ScreenController", result!.Rssi));
        }

        public override void OnScanFailed(ScanFailure errorCode) =>
            writer.TryComplete(new IOException($"Android Bluetooth scan failed: {errorCode}."));
    }

    private sealed class AndroidL2CapTransport(BluetoothSocket socket) : IControllerTransport
    {
        private readonly Stream input = socket.InputStream!;
        private readonly Stream output = socket.OutputStream!;
        public bool IsConnected => socket.IsConnected;
        public ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken token) => input.ReadAsync(buffer, token);

        public async ValueTask WriteAsync(ReadOnlyMemory<byte> buffer, IProgress<TransferProgress>? progress, CancellationToken token)
        {
            const int chunkSize = 64 * 1024;
            var sent = 0;
            while (sent < buffer.Length)
            {
                var length = Math.Min(chunkSize, buffer.Length - sent);
                await output.WriteAsync(buffer.Slice(sent, length), token);
                sent += length;
                progress?.Report(new(sent, buffer.Length));
            }
            await output.FlushAsync(token);
        }

        public ValueTask DisposeAsync()
        {
            try { socket.Close(); } finally { input.Dispose(); output.Dispose(); socket.Dispose(); }
            return ValueTask.CompletedTask;
        }
    }
}
