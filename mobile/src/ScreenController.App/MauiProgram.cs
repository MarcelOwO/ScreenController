using ScreenController.App.Services;
using ScreenController.App.ViewModels;
using ScreenController.App.Views;
using ScreenController.Protocol;

namespace ScreenController.App;

public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder.UseMauiApp<ApplicationRoot>();
        builder.Services.AddSingleton<IControllerTransportFactory, PlatformBluetoothTransportFactory>();
        builder.Services.AddSingleton<ScreenControllerClient>();
        builder.Services.AddSingleton<EnrollmentStore>();
        builder.Services.AddSingleton<ControllerViewModel>();
        builder.Services.AddSingleton<DashboardPage>();
        return builder.Build();
    }
}
