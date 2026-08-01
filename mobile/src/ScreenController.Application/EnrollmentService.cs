using Microsoft.Extensions.Logging;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;

namespace ScreenController.Application;

public sealed class EnrollmentService(
    IEnrollmentRepository repository,
    ILogger<EnrollmentService> logger)
{
    public async Task<DeviceEnrollment?> LoadAsync(CancellationToken cancellationToken = default)
    {
        var enrollment = await repository.LoadAsync(cancellationToken).ConfigureAwait(false);
        logger.LogInformation("Enrollment state loaded. Configured: {IsConfigured}", enrollment is not null);
        return enrollment;
    }

    public async Task<DeviceEnrollment> EnrollAsync(
        string enrollmentUri,
        DeviceCandidate device,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(device);
        var parsed = DeviceEnrollment.Parse(enrollmentUri);
        var enrollment = parsed with { DeviceId = device.Id, DisplayName = device.Name };
        await repository.SaveAsync(enrollment, cancellationToken).ConfigureAwait(false);
        logger.LogInformation("Controller enrollment saved for device {DeviceId}", device.Id);
        return enrollment;
    }

    public async Task ForgetAsync(CancellationToken cancellationToken = default)
    {
        await repository.RemoveAsync(cancellationToken).ConfigureAwait(false);
        logger.LogInformation("Controller enrollment removed");
    }
}
