using System.Diagnostics;
using System.Text.Json;
using System.Text.Json.Serialization;
using Microsoft.Extensions.Logging;

namespace ScreenController.Infrastructure.Logging;

public sealed class JsonFileLoggerProvider : ILoggerProvider, ISupportExternalScope
{
    private readonly Lock syncRoot = new();
    private readonly string directory;
    private readonly string activePath;
    private readonly long maximumFileBytes;
    private readonly int retainedFileCount;
    private IExternalScopeProvider scopeProvider = new LoggerExternalScopeProvider();
    private bool disposed;

    public JsonFileLoggerProvider(
        string directory,
        long maximumFileBytes = 5 * 1024 * 1024,
        int retainedFileCount = 5)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(directory);
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(maximumFileBytes);
        ArgumentOutOfRangeException.ThrowIfNegative(retainedFileCount);
        this.directory = directory;
        this.maximumFileBytes = maximumFileBytes;
        this.retainedFileCount = retainedFileCount;
        activePath = Path.Combine(directory, "screen-controller.log");
    }

    public ILogger CreateLogger(string categoryName) => new JsonFileLogger(this, categoryName);

    public void SetScopeProvider(IExternalScopeProvider scopeProvider) =>
        this.scopeProvider = scopeProvider ?? throw new ArgumentNullException(nameof(scopeProvider));

    public void Dispose()
    {
        lock (syncRoot)
        {
            disposed = true;
        }
    }

    internal void Write<TState>(
        string category,
        LogLevel level,
        EventId eventId,
        TState state,
        Exception? exception,
        Func<TState, Exception?, string> formatter)
    {
        lock (syncRoot)
        {
            if (disposed)
            {
                return;
            }

            try
            {
                Directory.CreateDirectory(directory);
                RotateIfRequired();
                var scopes = new List<string>();
                scopeProvider.ForEachScope(static (scope, target) => target.Add(scope?.ToString() ?? string.Empty), scopes);
                var entry = new LogEntry(
                    DateTimeOffset.UtcNow,
                    level.ToString(),
                    category,
                    eventId.Id,
                    formatter(state, exception),
                    exception?.ToString(),
                    scopes.Count == 0 ? null : scopes);
                File.AppendAllText(activePath, JsonSerializer.Serialize(entry, LoggerJsonContext.Default.LogEntry) + Environment.NewLine);
            }
            catch (Exception loggingException) when (loggingException is IOException or UnauthorizedAccessException)
            {
                Debug.WriteLine($"ScreenController file logging failed: {loggingException}");
            }
        }
    }

    private void RotateIfRequired()
    {
        if (!File.Exists(activePath) || new FileInfo(activePath).Length < maximumFileBytes)
        {
            return;
        }

        if (retainedFileCount == 0)
        {
            File.Delete(activePath);
            return;
        }

        var oldest = $"{activePath}.{retainedFileCount}";
        if (File.Exists(oldest))
        {
            File.Delete(oldest);
        }

        for (var index = retainedFileCount - 1; index >= 1; index--)
        {
            var source = $"{activePath}.{index}";
            if (File.Exists(source))
            {
                File.Move(source, $"{activePath}.{index + 1}");
            }
        }

        File.Move(activePath, $"{activePath}.1");
    }

    private sealed class JsonFileLogger(JsonFileLoggerProvider provider, string category) : ILogger
    {
        public IDisposable? BeginScope<TState>(TState state) where TState : notnull =>
            provider.scopeProvider.Push(state);

        public bool IsEnabled(LogLevel logLevel) => logLevel != LogLevel.None;

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception? exception,
            Func<TState, Exception?, string> formatter)
        {
            ArgumentNullException.ThrowIfNull(formatter);
            if (IsEnabled(logLevel))
            {
                provider.Write(category, logLevel, eventId, state, exception, formatter);
            }
        }
    }
}

internal sealed record LogEntry(
    DateTimeOffset TimestampUtc,
    string Level,
    string Category,
    int EventId,
    string Message,
    string? Exception,
    IReadOnlyList<string>? Scopes);

[System.Text.Json.Serialization.JsonSerializable(typeof(LogEntry))]
internal sealed partial class LoggerJsonContext : JsonSerializerContext;
