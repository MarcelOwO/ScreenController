using ScreenController.Domain;

namespace ScreenController.Application.Abstractions;

public interface IDisplayController : IAsyncDisposable
{
    bool IsConnected { get; }

    event EventHandler<bool>? ConnectionChanged;

    IAsyncEnumerable<DeviceCandidate> ScanAsync(CancellationToken cancellationToken);
    Task ConnectAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken);
    Task DisconnectAsync();
    Task<IReadOnlyList<string>> GetFilesAsync(CancellationToken cancellationToken);
    Task<DisplayStatus> GetStatusAsync(CancellationToken cancellationToken);
    Task RotateAsync(CancellationToken cancellationToken);
    Task SelectAsync(string fileName, CancellationToken cancellationToken);
    Task DeleteAsync(string fileName, CancellationToken cancellationToken);
    Task SetBrightnessAsync(int brightness, CancellationToken cancellationToken);
    Task SetScreenEnabledAsync(bool enabled, CancellationToken cancellationToken);
    Task UploadAsync(
        string fileName,
        ReadOnlyMemory<byte> file,
        IProgress<TransferProgress>? progress,
        CancellationToken cancellationToken);
}
