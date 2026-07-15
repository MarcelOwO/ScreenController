using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ScreenController.App.Services;
using ScreenController.Protocol;

namespace ScreenController.App.ViewModels;

public sealed partial class ControllerViewModel : ObservableObject
{
    private readonly ScreenControllerClient client;
    private readonly EnrollmentStore store;
    private DeviceEnrollment? enrollment;

    public ControllerViewModel(ScreenControllerClient client, EnrollmentStore store)
    {
        this.client = client;
        this.store = store;
        client.ConnectionChanged += (_, connected) => MainThread.BeginInvokeOnMainThread(() =>
        {
            IsConnected = connected;
            ConnectionText = connected ? "Connected and authenticated" : "Disconnected";
        });
    }

    public ObservableCollection<DeviceCandidate> Devices { get; } = [];
    public ObservableCollection<string> Files { get; } = [];
    [ObservableProperty] private DeviceCandidate? selectedDevice;
    [ObservableProperty] private string enrollmentText = string.Empty;
    [ObservableProperty] private string connectionText = "Not configured";
    [ObservableProperty] private string activityText = "Ready";
    [ObservableProperty] private bool isConnected;
    [ObservableProperty] private bool isBusy;
    [ObservableProperty] private bool displayEnabled = true;
    [ObservableProperty] private double brightness = 100;
    [ObservableProperty] private double uploadProgress;
    [ObservableProperty] private bool hasEnrollment;
    public bool CanControl => IsConnected && !IsBusy;
    partial void OnIsConnectedChanged(bool value) => OnPropertyChanged(nameof(CanControl));
    partial void OnIsBusyChanged(bool value) => OnPropertyChanged(nameof(CanControl));

    [RelayCommand]
    private async Task InitializeAsync()
    {
        enrollment = await store.LoadAsync();
        HasEnrollment = enrollment is not null;
        ConnectionText = enrollment is null ? "Enroll this controller to begin" : $"Saved: {enrollment.DisplayName}";
    }

    [RelayCommand]
    private async Task ScanAsync()
    {
        Devices.Clear();
        IsBusy = true;
        ActivityText = "Scanning for nearby displays…";
        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(12));
        try
        {
            await foreach (var device in client.ScanAsync(timeout.Token))
            {
                if (Devices.All(item => item.Id != device.Id)) Devices.Add(device);
            }
        }
        catch (OperationCanceledException) { }
        catch (Exception exception) { await ErrorAsync("Bluetooth scan failed", exception); }
        finally { IsBusy = false; ActivityText = Devices.Count == 0 ? "No display found" : "Select your display"; }
    }

    [RelayCommand]
    private async Task SaveEnrollmentAsync()
    {
        if (SelectedDevice is null) { await AlertAsync("Select a display", "Scan and select the Raspberry Pi first."); return; }
        try
        {
            var parsed = DeviceEnrollment.Parse(EnrollmentText);
            enrollment = parsed with { DeviceId = SelectedDevice.Id, DisplayName = SelectedDevice.Name };
            await store.SaveAsync(enrollment);
            HasEnrollment = true;
            EnrollmentText = string.Empty;
            ConnectionText = $"Saved: {enrollment.DisplayName}";
        }
        catch (Exception exception) { await ErrorAsync("Enrollment failed", exception); }
    }

    [RelayCommand]
    private async Task PasteEnrollmentAsync()
    {
        if (Clipboard.Default.HasText)
        {
            EnrollmentText = await Clipboard.Default.GetTextAsync() ?? string.Empty;
        }
    }

    [RelayCommand]
    private async Task ConnectAsync()
    {
        enrollment ??= await store.LoadAsync();
        if (enrollment is null) { await AlertAsync("Enrollment required", "Add the private enrollment URI first."); return; }
        IsBusy = true;
        ActivityText = "Connecting securely…";
        try { await client.ConnectAsync(enrollment, CancellationToken.None); await RefreshCoreAsync(CancellationToken.None); ActivityText = "Ready"; }
        catch (Exception exception) { await ErrorAsync("Connection failed", exception); }
        finally { IsBusy = false; }
    }

    [RelayCommand] private async Task DisconnectAsync() { await client.DisconnectAsync(); ActivityText = "Disconnected"; }
    [RelayCommand] private Task RefreshAsync() => RunAsync("Refreshing…", RefreshCoreAsync);
    [RelayCommand] private Task RotateAsync() => RunAsync("Rotating…", client.RotateAsync);
    [RelayCommand] private Task ApplyBrightnessAsync() => RunAsync("Updating brightness…", token => client.SetBrightnessAsync((int)Math.Round(Brightness), token));

    [RelayCommand]
    private Task ToggleDisplayAsync() => RunAsync(DisplayEnabled ? "Turning display off…" : "Turning display on…", async token =>
    {
        await client.SetScreenEnabledAsync(!DisplayEnabled, token);
        DisplayEnabled = !DisplayEnabled;
    });

    [RelayCommand]
    private Task SelectFileAsync(string? fileName) => string.IsNullOrWhiteSpace(fileName)
        ? Task.CompletedTask : RunAsync($"Displaying {fileName}…", token => client.SelectAsync(fileName, token));

    [RelayCommand]
    private async Task DeleteFileAsync(string? fileName)
    {
        if (string.IsNullOrWhiteSpace(fileName)) return;
        if (!await CurrentPage.DisplayAlertAsync("Delete media?", $"Delete {fileName} from the Pi?", "Delete", "Cancel")) return;
        await RunAsync($"Deleting {fileName}…", async token => { await client.DeleteAsync(fileName, token); await RefreshFilesAsync(token); });
    }

    [RelayCommand]
    private async Task UploadAsync()
    {
        var picked = await FilePicker.Default.PickAsync(new PickOptions { PickerTitle = "Choose an image or video" });
        if (picked is null) return;
        await RunAsync($"Uploading {picked.FileName}…", async token =>
        {
            await using var input = await picked.OpenReadAsync();
            using var data = new MemoryStream();
            await input.CopyToAsync(data, token);
            UploadProgress = 0;
            await client.UploadAsync(picked.FileName, data.ToArray(),
                new Progress<TransferProgress>(value => UploadProgress = value.Fraction), token);
            UploadProgress = 1;
            await RefreshFilesAsync(token);
        });
    }

    [RelayCommand]
    private async Task ForgetControllerAsync()
    {
        await client.DisconnectAsync(); store.Remove(); enrollment = null; HasEnrollment = false;
        Files.Clear(); ConnectionText = "Not configured"; ActivityText = "Enrollment removed";
    }

    private async Task RefreshCoreAsync(CancellationToken token)
    {
        var status = await client.GetStatusAsync(token);
        Brightness = status.Brightness; DisplayEnabled = status.DisplayEnabled;
        await RefreshFilesAsync(token);
    }

    private async Task RefreshFilesAsync(CancellationToken token)
    {
        var files = await client.GetFilesAsync(token); Files.Clear();
        foreach (var file in files) Files.Add(file);
    }

    private async Task RunAsync(string activity, Func<CancellationToken, Task> action)
    {
        if (!IsConnected) return;
        IsBusy = true; ActivityText = activity;
        try { using var timeout = new CancellationTokenSource(TimeSpan.FromMinutes(5)); await action(timeout.Token); ActivityText = "Ready"; }
        catch (Exception exception) { await ErrorAsync("Operation failed", exception); }
        finally { IsBusy = false; }
    }

    private async Task ErrorAsync(string title, Exception exception) { ActivityText = exception.Message; await AlertAsync(title, exception.Message); }
    private static Page CurrentPage => Application.Current!.Windows[0].Page!;
    private static Task AlertAsync(string title, string message) => CurrentPage.DisplayAlertAsync(title, message, "OK");
}
