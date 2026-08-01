using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Extensions.Logging;
using ScreenController.Application;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;

namespace ScreenController.Presentation;

public sealed partial class ControllerViewModel : ObservableObject, IDisposable
{
    private const int MaximumUploadBytes = 128 * 1024 * 1024;
    private readonly IDisplayController controller;
    private readonly EnrollmentService enrollmentService;
    private readonly IClipboardService clipboard;
    private readonly IUserDialogService dialogs;
    private readonly IMediaPickerService mediaPicker;
    private readonly IUiDispatcher dispatcher;
    private readonly ILogger<ControllerViewModel> logger;
    private DeviceEnrollment? enrollment;
    private bool initialized;
    private bool disposed;

    public ControllerViewModel(
        IDisplayController controller,
        EnrollmentService enrollmentService,
        IClipboardService clipboard,
        IUserDialogService dialogs,
        IMediaPickerService mediaPicker,
        IUiDispatcher dispatcher,
        ILogger<ControllerViewModel> logger)
    {
        this.controller = controller;
        this.enrollmentService = enrollmentService;
        this.clipboard = clipboard;
        this.dialogs = dialogs;
        this.mediaPicker = mediaPicker;
        this.dispatcher = dispatcher;
        this.logger = logger;
        controller.ConnectionChanged += OnConnectionChanged;
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
        if (initialized)
        {
            return;
        }

        try
        {
            enrollment = await enrollmentService.LoadAsync();
            HasEnrollment = enrollment is not null;
            ConnectionText = enrollment is null
                ? "Enroll this controller to begin"
                : $"Saved: {enrollment.DisplayName}";
            initialized = true;
        }
        catch (Exception exception)
        {
            await ErrorAsync("Could not load controller settings", exception);
        }
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
            await foreach (var device in controller.ScanAsync(timeout.Token))
            {
                if (Devices.All(item => item.Id != device.Id))
                {
                    Devices.Add(device);
                }
            }
        }
        catch (OperationCanceledException) when (timeout.IsCancellationRequested)
        {
            logger.LogInformation("Bluetooth scan window completed with {DeviceCount} devices", Devices.Count);
        }
        catch (Exception exception)
        {
            await ErrorAsync("Bluetooth scan failed", exception);
        }
        finally
        {
            IsBusy = false;
            ActivityText = Devices.Count == 0 ? "No display found" : "Select your display";
        }
    }

    [RelayCommand]
    private async Task SaveEnrollmentAsync()
    {
        if (SelectedDevice is null)
        {
            await dialogs.ShowErrorAsync("Select a display", "Scan and select the Raspberry Pi first.");
            return;
        }

        try
        {
            enrollment = await enrollmentService.EnrollAsync(EnrollmentText, SelectedDevice);
            HasEnrollment = true;
            EnrollmentText = string.Empty;
            ConnectionText = $"Saved: {enrollment.DisplayName}";
            ActivityText = "Controller enrolled";
        }
        catch (Exception exception)
        {
            await ErrorAsync("Enrollment failed", exception);
        }
    }

    [RelayCommand]
    private async Task PasteEnrollmentAsync() =>
        EnrollmentText = await clipboard.GetTextAsync() ?? string.Empty;

    [RelayCommand]
    private async Task ConnectAsync()
    {
        enrollment ??= await enrollmentService.LoadAsync();
        if (enrollment is null)
        {
            await dialogs.ShowErrorAsync("Enrollment required", "Add the private enrollment URI first.");
            return;
        }

        IsBusy = true;
        ActivityText = "Connecting securely…";
        try
        {
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(30));
            await controller.ConnectAsync(enrollment, timeout.Token);
            await RefreshCoreAsync(timeout.Token);
            ActivityText = "Ready";
        }
        catch (Exception exception)
        {
            await ErrorAsync("Connection failed", exception);
        }
        finally
        {
            IsBusy = false;
        }
    }

    [RelayCommand]
    private async Task DisconnectAsync()
    {
        try
        {
            await controller.DisconnectAsync();
            ActivityText = "Disconnected";
        }
        catch (Exception exception)
        {
            await ErrorAsync("Disconnect failed", exception);
        }
    }

    [RelayCommand] private Task RefreshAsync() => RunAsync("Refreshing…", RefreshCoreAsync);
    [RelayCommand] private Task RotateAsync() => RunAsync("Rotating…", controller.RotateAsync);
    [RelayCommand] private Task ApplyBrightnessAsync() => RunAsync(
        "Updating brightness…",
        token => controller.SetBrightnessAsync((int)Math.Round(Brightness), token));

    [RelayCommand]
    private Task ToggleDisplayAsync() => RunAsync(
        DisplayEnabled ? "Turning display off…" : "Turning display on…",
        async token =>
        {
            await controller.SetScreenEnabledAsync(!DisplayEnabled, token);
            DisplayEnabled = !DisplayEnabled;
        });

    [RelayCommand]
    private Task SelectFileAsync(string? fileName) => string.IsNullOrWhiteSpace(fileName)
        ? Task.CompletedTask
        : RunAsync($"Displaying {fileName}…", token => controller.SelectAsync(fileName, token));

    [RelayCommand]
    private async Task DeleteFileAsync(string? fileName)
    {
        if (string.IsNullOrWhiteSpace(fileName) ||
            !await dialogs.ConfirmAsync("Delete media?", $"Delete {fileName} from the Pi?", "Delete", "Cancel"))
        {
            return;
        }

        await RunAsync($"Deleting {fileName}…", async token =>
        {
            await controller.DeleteAsync(fileName, token);
            await RefreshFilesAsync(token);
        });
    }

    [RelayCommand]
    private async Task UploadAsync()
    {
        var picked = await mediaPicker.PickAsync();
        if (picked is null)
        {
            return;
        }

        await RunAsync($"Uploading {picked.FileName}…", async token =>
        {
            await using var input = await picked.OpenReadAsync(token);
            var bytes = await ReadMediaAsync(input, token);
            UploadProgress = 0;
            await controller.UploadAsync(
                picked.FileName,
                bytes,
                new Progress<TransferProgress>(value => dispatcher.Dispatch(() => UploadProgress = value.Fraction)),
                token);
            UploadProgress = 1;
            await RefreshFilesAsync(token);
        });
    }

    [RelayCommand]
    private async Task ForgetControllerAsync()
    {
        try
        {
            await controller.DisconnectAsync();
            await enrollmentService.ForgetAsync();
            enrollment = null;
            HasEnrollment = false;
            Files.Clear();
            ConnectionText = "Not configured";
            ActivityText = "Enrollment removed";
        }
        catch (Exception exception)
        {
            await ErrorAsync("Could not forget controller", exception);
        }
    }

    public void Dispose()
    {
        if (disposed)
        {
            return;
        }

        controller.ConnectionChanged -= OnConnectionChanged;
        disposed = true;
    }

    private void OnConnectionChanged(object? sender, bool connected) => dispatcher.Dispatch(() =>
    {
        IsConnected = connected;
        ConnectionText = connected ? "Connected and authenticated" : "Disconnected";
    });

    private async Task RefreshCoreAsync(CancellationToken token)
    {
        var status = await controller.GetStatusAsync(token);
        Brightness = status.Brightness;
        DisplayEnabled = status.DisplayEnabled;
        await RefreshFilesAsync(token);
    }

    private async Task RefreshFilesAsync(CancellationToken token)
    {
        var files = await controller.GetFilesAsync(token);
        Files.Clear();
        foreach (var file in files)
        {
            Files.Add(file);
        }
    }

    private async Task RunAsync(string activity, Func<CancellationToken, Task> action)
    {
        if (!IsConnected || IsBusy)
        {
            return;
        }

        IsBusy = true;
        ActivityText = activity;
        try
        {
            using var timeout = new CancellationTokenSource(TimeSpan.FromMinutes(5));
            await action(timeout.Token);
            ActivityText = "Ready";
        }
        catch (Exception exception)
        {
            await ErrorAsync("Operation failed", exception);
        }
        finally
        {
            IsBusy = false;
        }
    }

    private async Task ErrorAsync(string title, Exception exception)
    {
        logger.LogError(exception, "{Operation} in controller UI", title);
        ActivityText = exception.Message;
        await dialogs.ShowErrorAsync(title, exception.Message);
    }

    private static async Task<byte[]> ReadMediaAsync(Stream input, CancellationToken cancellationToken)
    {
        using var data = new MemoryStream();
        var buffer = new byte[80 * 1024];
        while (true)
        {
            var read = await input.ReadAsync(buffer, cancellationToken);
            if (read == 0)
            {
                return data.ToArray();
            }

            if (data.Length + read > MaximumUploadBytes)
            {
                throw new InvalidDataException("Files larger than 128 MiB are not supported.");
            }

            await data.WriteAsync(buffer.AsMemory(0, read), cancellationToken);
        }
    }
}
