using ScreenController.App.ViewModels;

namespace ScreenController.App.Views;

public sealed partial class DashboardPage : ContentPage
{
    private readonly ControllerViewModel viewModel;
    public DashboardPage(ControllerViewModel viewModel) { InitializeComponent(); BindingContext = this.viewModel = viewModel; }
    protected override async void OnAppearing() { base.OnAppearing(); await viewModel.InitializeCommand.ExecuteAsync(null); }
}
