using Microsoft.Extensions.Logging;

namespace ScreenController.App.Services;

public sealed class StartupDiagnostics(ILogger<StartupDiagnostics> logger) : IDisposable
{
    private bool started;

    public void Start()
    {
        if (started)
        {
            return;
        }

        AppDomain.CurrentDomain.UnhandledException += OnUnhandledException;
        TaskScheduler.UnobservedTaskException += OnUnobservedTaskException;
        started = true;
        logger.LogInformation(
            "Diagnostics started on {OperatingSystem} with runtime {RuntimeVersion}",
            Environment.OSVersion,
            Environment.Version);
    }

    public void Dispose()
    {
        if (!started)
        {
            return;
        }

        AppDomain.CurrentDomain.UnhandledException -= OnUnhandledException;
        TaskScheduler.UnobservedTaskException -= OnUnobservedTaskException;
        started = false;
    }

    private void OnUnhandledException(object sender, UnhandledExceptionEventArgs args) =>
        logger.LogCritical(
            args.ExceptionObject as Exception,
            "Unhandled application exception. Process terminating: {IsTerminating}",
            args.IsTerminating);

    private void OnUnobservedTaskException(object? sender, UnobservedTaskExceptionEventArgs args)
    {
        logger.LogError(args.Exception, "Unobserved task exception");
        args.SetObserved();
    }
}
