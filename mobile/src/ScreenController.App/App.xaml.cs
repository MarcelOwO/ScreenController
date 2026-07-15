using ScreenController.App.Views;

namespace ScreenController.App;

public sealed partial class ApplicationRoot : Application
{
    private readonly DashboardPage dashboardPage;
    public ApplicationRoot(DashboardPage dashboardPage) { InitializeComponent(); this.dashboardPage = dashboardPage; }
    protected override Window CreateWindow(IActivationState? activationState) =>
        new(new NavigationPage(dashboardPage) { BarBackgroundColor = Color.FromArgb("#0B1020"), BarTextColor = Colors.White });
}
