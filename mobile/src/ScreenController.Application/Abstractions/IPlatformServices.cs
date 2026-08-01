namespace ScreenController.Application.Abstractions;

public interface IClipboardService
{
    Task<string?> GetTextAsync(CancellationToken cancellationToken = default);
}

public interface IUserDialogService
{
    Task ShowErrorAsync(string title, string message, CancellationToken cancellationToken = default);
    Task<bool> ConfirmAsync(string title, string message, string accept, string cancel,
        CancellationToken cancellationToken = default);
}

public interface IMediaPickerService
{
    Task<SelectedMedia?> PickAsync(CancellationToken cancellationToken = default);
}

public interface IUiDispatcher
{
    void Dispatch(Action action);
}

public sealed record SelectedMedia(string FileName, Func<CancellationToken, Task<Stream>> OpenReadAsync);
