using ScreenController.Domain;

namespace ScreenController.Application.Abstractions;

public interface IEnrollmentRepository
{
    Task<DeviceEnrollment?> LoadAsync(CancellationToken cancellationToken = default);
    Task SaveAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken = default);
    Task RemoveAsync(CancellationToken cancellationToken = default);
}

public interface ISecureValueStore
{
    Task<string?> GetAsync(string key, CancellationToken cancellationToken = default);
    Task SetAsync(string key, string value, CancellationToken cancellationToken = default);
    Task RemoveAsync(string key, CancellationToken cancellationToken = default);
}
