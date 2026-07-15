namespace ScreenController.App.WinUI;

public sealed partial class App : MauiWinUIApplication
{
    public App() => InitializeComponent();
    protected override MauiApp CreateMauiApp() => MauiProgram.CreateMauiApp();
}
