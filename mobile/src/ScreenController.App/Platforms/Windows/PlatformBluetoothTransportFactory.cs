using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Channels;
using ScreenController.Domain;
using ScreenController.Protocol;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;

namespace ScreenController.App.Services;

public sealed class PlatformBluetoothTransportFactory : IControllerTransportFactory
{
    public async IAsyncEnumerable<DeviceCandidate> ScanAsync(
        [System.Runtime.CompilerServices.EnumeratorCancellation] CancellationToken cancellationToken)
    {
        var results = Channel.CreateUnbounded<DeviceCandidate>();
        var watcher = new BluetoothLEAdvertisementWatcher { ScanningMode = BluetoothLEScanningMode.Active };
        watcher.AdvertisementFilter.Advertisement.ServiceUuids.Add(ProtocolConstants.ServiceUuid);
        watcher.Received += OnReceived;
        watcher.Stopped += OnStopped;
        watcher.Start();
        using var registration = cancellationToken.Register(() => results.Writer.TryComplete());
        try { await foreach (var result in results.Reader.ReadAllAsync(cancellationToken)) yield return result; }
        finally { watcher.Stop(); watcher.Received -= OnReceived; watcher.Stopped -= OnStopped; }

        void OnReceived(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args)
        {
            results.Writer.TryWrite(new(args.BluetoothAddress.ToString("X12"), "ScreenController", args.RawSignalStrengthInDBm));
        }
        void OnStopped(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementWatcherStoppedEventArgs args)
        {
            if (args.Error != BluetoothError.Success) results.Writer.TryComplete(new IOException($"Windows Bluetooth scan failed: {args.Error}."));
        }
    }

    public async Task<IControllerTransport> ConnectAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken)
    {
        var address = Convert.ToUInt64(enrollment.DeviceId, 16);
        var device = await BluetoothLEDevice.FromBluetoothAddressAsync(address).AsTask(cancellationToken)
            ?? throw new IOException("Windows could not open the saved Bluetooth device.");
        var services = await device.GetGattServicesForUuidAsync(ProtocolConstants.ServiceUuid, BluetoothCacheMode.Uncached)
            .AsTask(cancellationToken);
        if (services.Status != GattCommunicationStatus.Success || services.Services.Count == 0)
            throw new IOException($"ScreenController GATT service was unavailable: {services.Status}.");
        var service = services.Services[0];
        var writes = await service.GetCharacteristicsForUuidAsync(ProtocolConstants.GattWriteUuid, BluetoothCacheMode.Uncached)
            .AsTask(cancellationToken);
        var notifications = await service.GetCharacteristicsForUuidAsync(ProtocolConstants.GattNotifyUuid, BluetoothCacheMode.Uncached)
            .AsTask(cancellationToken);
        if (writes.Characteristics.Count == 0 || notifications.Characteristics.Count == 0)
            throw new IOException("ScreenController GATT characteristics are missing.");

        var transport = new WindowsGattTransport(device, service, writes.Characteristics[0], notifications.Characteristics[0]);
        await transport.InitializeAsync(cancellationToken);
        return transport;
    }

    private sealed class WindowsGattTransport(
        BluetoothLEDevice device,
        GattDeviceService service,
        GattCharacteristic write,
        GattCharacteristic notify) : IControllerTransport
    {
        private readonly Channel<byte[]> incoming = Channel.CreateUnbounded<byte[]>();
        private byte[] current = [];
        private int currentOffset;
        public bool IsConnected => device.ConnectionStatus == BluetoothConnectionStatus.Connected;

        public async Task InitializeAsync(CancellationToken token)
        {
            notify.ValueChanged += OnValueChanged;
            var result = await notify.WriteClientCharacteristicConfigurationDescriptorAsync(
                GattClientCharacteristicConfigurationDescriptorValue.Notify).AsTask(token);
            if (result != GattCommunicationStatus.Success) throw new IOException($"Could not subscribe to controller responses: {result}.");
            await WriteChunkAsync(new byte[] { 0x01 }, token); // Request the auth challenge.
        }

        public async ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken token)
        {
            if (currentOffset == current.Length) { current = await incoming.Reader.ReadAsync(token); currentOffset = 0; }
            var count = Math.Min(buffer.Length, current.Length - currentOffset);
            current.AsSpan(currentOffset, count).CopyTo(buffer.Span);
            currentOffset += count;
            return count;
        }

        public async ValueTask WriteAsync(ReadOnlyMemory<byte> buffer, IProgress<TransferProgress>? progress, CancellationToken token)
        {
            var mtuPayload = Math.Max(20, service.Session.MaxPduSize - 3);
            var sent = 0;
            while (sent < buffer.Length)
            {
                var length = Math.Min(mtuPayload, buffer.Length - sent);
                await WriteChunkAsync(buffer.Slice(sent, length), token);
                sent += length;
                progress?.Report(new(sent, buffer.Length));
            }
        }

        private async Task WriteChunkAsync(ReadOnlyMemory<byte> bytes, CancellationToken token)
        {
            using var writer = new DataWriter();
            writer.WriteBytes(bytes.ToArray());
            var status = await write.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse).AsTask(token);
            if (status != GattCommunicationStatus.Success) throw new IOException($"Bluetooth GATT write failed: {status}.");
        }

        private void OnValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            var reader = DataReader.FromBuffer(args.CharacteristicValue);
            var value = new byte[reader.UnconsumedBufferLength];
            reader.ReadBytes(value);
            incoming.Writer.TryWrite(value);
        }

        public ValueTask DisposeAsync()
        {
            notify.ValueChanged -= OnValueChanged;
            incoming.Writer.TryComplete();
            service.Dispose(); device.Dispose();
            return ValueTask.CompletedTask;
        }
    }
}
