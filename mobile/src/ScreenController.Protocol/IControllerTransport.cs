using ScreenController.Domain;

namespace ScreenController.Protocol;

public interface IControllerTransport : IAsyncDisposable
{
    bool IsConnected { get; }

    ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken cancellationToken);

    ValueTask WriteAsync(
        ReadOnlyMemory<byte> buffer,
        IProgress<TransferProgress>? progress,
        CancellationToken cancellationToken);
}

public interface IControllerTransportFactory
{
    IAsyncEnumerable<DeviceCandidate> ScanAsync(CancellationToken cancellationToken);

    Task<IControllerTransport> ConnectAsync(
        DeviceEnrollment enrollment,
        CancellationToken cancellationToken);
}
