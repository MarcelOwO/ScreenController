using System.Text.Json;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;
using ScreenController.Infrastructure.Logging;
using Xunit;

namespace ScreenController.Infrastructure.Tests;

public sealed class InfrastructureTests
{
    [Fact]
    public async Task SecureRepositoryRoundTripsEnrollment()
    {
        var store = new MemorySecureStore();
        var repository = new SecureEnrollmentRepository(store, NullLogger<SecureEnrollmentRepository>.Instance);
        var expected = new DeviceEnrollment("device", "Display", Enumerable.Repeat((byte)0xCC, 32).ToArray());

        await repository.SaveAsync(expected, TestContext.Current.CancellationToken);
        var actual = await repository.LoadAsync(TestContext.Current.CancellationToken);

        Assert.NotNull(actual);
        Assert.Equal(expected.DeviceId, actual.DeviceId);
        Assert.Equal(expected.DisplayName, actual.DisplayName);
        Assert.Equal(expected.Key, actual.Key);
        Assert.DoesNotContain(expected.ToEnrollmentUri(), store.Value, StringComparison.Ordinal);
    }

    [Fact]
    public async Task SecureRepositoryDiscardsCorruptValue()
    {
        var store = new MemorySecureStore { Value = "not-json" };
        var repository = new SecureEnrollmentRepository(store, NullLogger<SecureEnrollmentRepository>.Instance);

        var actual = await repository.LoadAsync(TestContext.Current.CancellationToken);

        Assert.Null(actual);
        Assert.Null(store.Value);
    }

    [Fact]
    public void FileLoggerWritesStructuredJsonAndRotates()
    {
        var directory = Path.Combine(Path.GetTempPath(), $"screen-controller-log-{Guid.NewGuid():N}");
        try
        {
            using var provider = new JsonFileLoggerProvider(directory, maximumFileBytes: 1, retainedFileCount: 2);
            var logger = provider.CreateLogger("Test.Category");

            logger.LogInformation("First event {Value}", 42);
            logger.LogWarning("Second event");

            var active = Path.Combine(directory, "screen-controller.log");
            Assert.True(File.Exists(active));
            Assert.True(File.Exists(active + ".1"));
            using var document = JsonDocument.Parse(File.ReadAllText(active));
            Assert.Equal("Warning", document.RootElement.GetProperty("Level").GetString());
            Assert.Equal("Test.Category", document.RootElement.GetProperty("Category").GetString());
        }
        finally
        {
            if (Directory.Exists(directory))
            {
                Directory.Delete(directory, recursive: true);
            }
        }
    }

    private sealed class MemorySecureStore : ISecureValueStore
    {
        public string? Value { get; set; }
        public Task<string?> GetAsync(string key, CancellationToken cancellationToken = default) => Task.FromResult(Value);
        public Task SetAsync(string key, string value, CancellationToken cancellationToken = default)
        {
            Value = value;
            return Task.CompletedTask;
        }
        public Task RemoveAsync(string key, CancellationToken cancellationToken = default)
        {
            Value = null;
            return Task.CompletedTask;
        }
    }
}
