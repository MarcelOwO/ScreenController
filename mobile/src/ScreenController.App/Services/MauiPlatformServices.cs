using ScreenController.Application.Abstractions;

namespace ScreenController.App.Services;

public sealed class MauiPlatformServices :
    ISecureValueStore,
    IClipboardService,
    IUserDialogService,
    IMediaPickerService,
    IUiDispatcher
{
    public async Task<string?> GetAsync(string key, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        return await SecureStorage.Default.GetAsync(key);
    }

    public async Task SetAsync(string key, string value, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        await SecureStorage.Default.SetAsync(key, value);
    }

    public Task RemoveAsync(string key, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        SecureStorage.Default.Remove(key);
        return Task.CompletedTask;
    }

    public async Task<string?> GetTextAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        return Clipboard.Default.HasText ? await Clipboard.Default.GetTextAsync() : null;
    }

    public Task ShowErrorAsync(string title, string message, CancellationToken cancellationToken = default) =>
        MainThread.InvokeOnMainThreadAsync(async () =>
        {
            cancellationToken.ThrowIfCancellationRequested();
            await GetCurrentPage().DisplayAlertAsync(title, message, "OK");
        });

    public Task<bool> ConfirmAsync(
        string title,
        string message,
        string accept,
        string cancel,
        CancellationToken cancellationToken = default) =>
        MainThread.InvokeOnMainThreadAsync(async () =>
        {
            cancellationToken.ThrowIfCancellationRequested();
            return await GetCurrentPage().DisplayAlertAsync(title, message, accept, cancel);
        });

    public async Task<SelectedMedia?> PickAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var result = await FilePicker.Default.PickAsync(new PickOptions
        {
            PickerTitle = "Choose an image or video",
        });
        return result is null
            ? null
            : new SelectedMedia(result.FileName, async token =>
            {
                token.ThrowIfCancellationRequested();
                return await result.OpenReadAsync();
            });
    }

    public void Dispatch(Action action)
    {
        ArgumentNullException.ThrowIfNull(action);
        if (MainThread.IsMainThread)
        {
            action();
        }
        else
        {
            MainThread.BeginInvokeOnMainThread(action);
        }
    }

    private static Page GetCurrentPage() =>
        Microsoft.Maui.Controls.Application.Current?.Windows.FirstOrDefault()?.Page
        ?? throw new InvalidOperationException("The application window is not available.");
}
