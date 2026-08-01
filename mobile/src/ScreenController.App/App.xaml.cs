using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using ScreenController.App.Services;
using ScreenController.App.Views;

namespace ScreenController.App;

public sealed partial class ApplicationRoot : Microsoft.Maui.Controls.Application
{
    private readonly IServiceProvider services;
    private readonly ILogger<ApplicationRoot> logger;

    public ApplicationRoot(
        IServiceProvider services,
        ILogger<ApplicationRoot> logger,
        StartupDiagnostics startupDiagnostics)
    {
        this.services = services;
        this.logger = logger;
        InitializeComponent();
        startupDiagnostics.Start();
        logger.LogInformation("Application resources initialized");
    }

    protected override Window CreateWindow(IActivationState? activationState)
    {
        try
        {
            var dashboardPage = services.GetRequiredService<DashboardPage>();
            var navigationPage = new NavigationPage(dashboardPage)
            {
                BarBackgroundColor = Color.FromArgb("#0B1020"),
                BarTextColor = Colors.White,
            };
            logger.LogInformation("Primary application window created");
            return new Window(navigationPage);
        }
        catch (Exception exception)
        {
            logger.LogCritical(exception, "Primary application window could not be created");
            return new Window(CreateStartupErrorPage());
        }
    }

    private static ContentPage CreateStartupErrorPage() => new()
    {
        BackgroundColor = Color.FromArgb("#0B1020"),
        Content = new VerticalStackLayout
        {
            Padding = 32,
            Spacing = 12,
            VerticalOptions = LayoutOptions.Center,
            Children =
            {
                new Label
                {
                    Text = "Screen Controller could not start",
                    FontSize = 26,
                    FontAttributes = FontAttributes.Bold,
                    TextColor = Colors.White,
                },
                new Label
                {
                    Text = "A diagnostic log was saved. Restart the app; if this continues, share the latest log with support.",
                    TextColor = Color.FromArgb("#AEB8D0"),
                },
            },
        },
    };
}
