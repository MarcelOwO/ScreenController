using Microsoft.Extensions.Logging.Abstractions;
using ScreenController.Application;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;
using Xunit;

namespace ScreenController.Presentation.Tests;

public sealed class ControllerViewModelTests
{
    [Fact]
    public async Task InitializeShowsSavedEnrollment()
    {
        var fixture = new Fixture
        {
            Enrollment = new DeviceEnrollment("id", "Studio", new byte[32]),
        };
        using var viewModel = fixture.CreateViewModel();

        await viewModel.InitializeCommand.ExecuteAsync(null);

        Assert.True(viewModel.HasEnrollment);
        Assert.Equal("Saved: Studio", viewModel.ConnectionText);
    }

    [Fact]
    public async Task SavingWithoutDeviceReportsValidationError()
    {
        var fixture = new Fixture();
        using var viewModel = fixture.CreateViewModel();

        await viewModel.SaveEnrollmentCommand.ExecuteAsync(null);

        Assert.Equal("Select a display", Assert.Single(fixture.Dialogs.Errors).Title);
    }

    [Fact]
    public async Task SavingEnrollmentUpdatesPresentationAndRepository()
    {
        var fixture = new Fixture();
        using var viewModel = fixture.CreateViewModel();
        viewModel.SelectedDevice = new DeviceCandidate("device", "Kitchen", -55);
        viewModel.EnrollmentText = new DeviceEnrollment("", "", new byte[32]).ToEnrollmentUri();

        await viewModel.SaveEnrollmentCommand.ExecuteAsync(null);

        Assert.True(viewModel.HasEnrollment);
        Assert.Equal("device", fixture.Enrollment?.DeviceId);
        Assert.Equal("Saved: Kitchen", viewModel.ConnectionText);
    }

    [Fact]
    public void ConnectionEventUpdatesControllableStateThroughDispatcher()
    {
        var fixture = new Fixture();
        using var viewModel = fixture.CreateViewModel();

        fixture.Controller.SetConnected(true);

        Assert.True(viewModel.IsConnected);
        Assert.True(viewModel.CanControl);
        Assert.Equal(1, fixture.Dispatcher.DispatchCount);
    }

    [Fact]
    public async Task ToggleDisplayCallsControllerAndUpdatesState()
    {
        var fixture = new Fixture();
        using var viewModel = fixture.CreateViewModel();
        fixture.Controller.SetConnected(true);

        await viewModel.ToggleDisplayCommand.ExecuteAsync(null);

        Assert.False(fixture.Controller.LastScreenEnabled);
        Assert.False(viewModel.DisplayEnabled);
        Assert.Equal("Ready", viewModel.ActivityText);
    }

    private sealed class Fixture : IEnrollmentRepository
    {
        public FakeController Controller { get; } = new();
        public FakeDialogs Dialogs { get; } = new();
        public InlineDispatcher Dispatcher { get; } = new();
        public DeviceEnrollment? Enrollment { get; set; }

        public ControllerViewModel CreateViewModel() => new(
            Controller,
            new EnrollmentService(this, NullLogger<EnrollmentService>.Instance),
            new EmptyClipboard(),
            Dialogs,
            new EmptyMediaPicker(),
            Dispatcher,
            NullLogger<ControllerViewModel>.Instance);

        public Task<DeviceEnrollment?> LoadAsync(CancellationToken cancellationToken = default) => Task.FromResult(Enrollment);
        public Task SaveAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken = default)
        {
            Enrollment = enrollment;
            return Task.CompletedTask;
        }
        public Task RemoveAsync(CancellationToken cancellationToken = default)
        {
            Enrollment = null;
            return Task.CompletedTask;
        }
    }

    private sealed class FakeController : IDisplayController
    {
        public bool IsConnected { get; private set; }
        public bool LastScreenEnabled { get; private set; } = true;
        public event EventHandler<bool>? ConnectionChanged;
        public void SetConnected(bool value)
        {
            IsConnected = value;
            ConnectionChanged?.Invoke(this, value);
        }
        public async IAsyncEnumerable<DeviceCandidate> ScanAsync(
            [System.Runtime.CompilerServices.EnumeratorCancellation] CancellationToken cancellationToken)
        {
            await Task.CompletedTask;
            yield break;
        }
        public Task ConnectAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken) { SetConnected(true); return Task.CompletedTask; }
        public Task DisconnectAsync() { SetConnected(false); return Task.CompletedTask; }
        public Task<IReadOnlyList<string>> GetFilesAsync(CancellationToken cancellationToken) => Task.FromResult<IReadOnlyList<string>>([]);
        public Task<DisplayStatus> GetStatusAsync(CancellationToken cancellationToken) => Task.FromResult(new DisplayStatus(100, true));
        public Task RotateAsync(CancellationToken cancellationToken) => Task.CompletedTask;
        public Task SelectAsync(string fileName, CancellationToken cancellationToken) => Task.CompletedTask;
        public Task DeleteAsync(string fileName, CancellationToken cancellationToken) => Task.CompletedTask;
        public Task SetBrightnessAsync(int brightness, CancellationToken cancellationToken) => Task.CompletedTask;
        public Task SetScreenEnabledAsync(bool enabled, CancellationToken cancellationToken) { LastScreenEnabled = enabled; return Task.CompletedTask; }
        public Task UploadAsync(string fileName, ReadOnlyMemory<byte> file, IProgress<TransferProgress>? progress, CancellationToken cancellationToken) => Task.CompletedTask;
        public ValueTask DisposeAsync() => ValueTask.CompletedTask;
    }

    private sealed class FakeDialogs : IUserDialogService
    {
        public List<(string Title, string Message)> Errors { get; } = [];
        public Task ShowErrorAsync(string title, string message, CancellationToken cancellationToken = default)
        {
            Errors.Add((title, message));
            return Task.CompletedTask;
        }
        public Task<bool> ConfirmAsync(string title, string message, string accept, string cancel, CancellationToken cancellationToken = default) => Task.FromResult(true);
    }

    private sealed class InlineDispatcher : IUiDispatcher
    {
        public int DispatchCount { get; private set; }
        public void Dispatch(Action action) { DispatchCount++; action(); }
    }

    private sealed class EmptyClipboard : IClipboardService
    {
        public Task<string?> GetTextAsync(CancellationToken cancellationToken = default) => Task.FromResult<string?>(null);
    }

    private sealed class EmptyMediaPicker : IMediaPickerService
    {
        public Task<SelectedMedia?> PickAsync(CancellationToken cancellationToken = default) => Task.FromResult<SelectedMedia?>(null);
    }
}
