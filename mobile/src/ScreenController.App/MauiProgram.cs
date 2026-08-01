using Microsoft.Extensions.Logging;
using ScreenController.Application;
using ScreenController.Application.Abstractions;
using ScreenController.App.Services;
using ScreenController.Infrastructure;
using ScreenController.Infrastructure.Logging;
using ScreenController.Presentation;
using ScreenController.App.Views;
using ScreenController.Protocol;

namespace ScreenController.App;

public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder.UseMauiApp<ApplicationRoot>();

        var logDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "ScreenController",
            "Logs");
        builder.Logging.SetMinimumLevel(LogLevel.Information);
        builder.Logging.AddProvider(new JsonFileLoggerProvider(logDirectory));

        builder.Services
            .AddScreenControllerApplication()
            .AddScreenControllerInfrastructure()
            .AddScreenControllerPresentation();

        builder.Services.AddSingleton<IControllerTransportFactory, PlatformBluetoothTransportFactory>();
        builder.Services.AddSingleton<ScreenControllerClient>();
        builder.Services.AddSingleton<IDisplayController>(services =>
            services.GetRequiredService<ScreenControllerClient>());
        builder.Services.AddSingleton<MauiPlatformServices>();
        builder.Services.AddSingleton<ISecureValueStore>(services => services.GetRequiredService<MauiPlatformServices>());
        builder.Services.AddSingleton<IClipboardService>(services => services.GetRequiredService<MauiPlatformServices>());
        builder.Services.AddSingleton<IUserDialogService>(services => services.GetRequiredService<MauiPlatformServices>());
        builder.Services.AddSingleton<IMediaPickerService>(services => services.GetRequiredService<MauiPlatformServices>());
        builder.Services.AddSingleton<IUiDispatcher>(services => services.GetRequiredService<MauiPlatformServices>());
        builder.Services.AddSingleton<StartupDiagnostics>();
        builder.Services.AddSingleton<DashboardPage>();
        return builder.Build();
    }
}
