using Microsoft.Extensions.Logging.Abstractions;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;
using Xunit;

namespace ScreenController.Application.Tests;

public sealed class EnrollmentServiceTests
{
    private readonly InMemoryEnrollmentRepository repository = new();

    [Fact]
    public async Task EnrollBindsPrivateUriToSelectedDevice()
    {
        var service = CreateService();
        var candidate = new DeviceCandidate("bluetooth-id", "Office", -40);
        var uri = new DeviceEnrollment(
            "uri-id",
            "URI name",
            Enumerable.Repeat((byte)0xA5, 32).ToArray()).ToEnrollmentUri();

        var result = await service.EnrollAsync(uri, candidate, TestContext.Current.CancellationToken);

        Assert.Equal(candidate.Id, result.DeviceId);
        Assert.Equal(candidate.Name, result.DisplayName);
        Assert.Same(result, repository.Value);
    }

    [Fact]
    public async Task ForgetRemovesEnrollment()
    {
        repository.Value = new("id", "name", new byte[32]);

        await CreateService().ForgetAsync(TestContext.Current.CancellationToken);

        Assert.Null(repository.Value);
    }

    [Fact]
    public async Task LoadReturnsRepositoryValue()
    {
        repository.Value = new("id", "name", new byte[32]);

        Assert.Same(repository.Value, await CreateService().LoadAsync(TestContext.Current.CancellationToken));
    }

    private EnrollmentService CreateService() =>
        new(repository, NullLogger<EnrollmentService>.Instance);

    private sealed class InMemoryEnrollmentRepository : IEnrollmentRepository
    {
        public DeviceEnrollment? Value { get; set; }
        public Task<DeviceEnrollment?> LoadAsync(CancellationToken cancellationToken = default) => Task.FromResult(Value);
        public Task SaveAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken = default)
        {
            Value = enrollment;
            return Task.CompletedTask;
        }
        public Task RemoveAsync(CancellationToken cancellationToken = default)
        {
            Value = null;
            return Task.CompletedTask;
        }
    }
}
