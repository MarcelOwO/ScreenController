using Microsoft.Extensions.Logging;
using ScreenController.Presentation;

namespace ScreenController.App.Views;

public sealed partial class DashboardPage : ContentPage
{
    private readonly ControllerViewModel viewModel;
    private readonly ILogger<DashboardPage> logger;

    public DashboardPage(ControllerViewModel viewModel, ILogger<DashboardPage> logger)
    {
        InitializeComponent();
        BindingContext = this.viewModel = viewModel;
        this.logger = logger;
    }

    protected override async void OnAppearing()
    {
        base.OnAppearing();
        try
        {
            await viewModel.InitializeCommand.ExecuteAsync(null);
        }
        catch (Exception exception)
        {
            logger.LogError(exception, "Dashboard initialization failed");
        }
    }
}
