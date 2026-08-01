using System.Text.Json;
using System.Text.Json.Serialization;
using Microsoft.Extensions.Logging;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;

namespace ScreenController.Infrastructure;

public sealed class SecureEnrollmentRepository(
    ISecureValueStore secureStore,
    ILogger<SecureEnrollmentRepository> logger) : IEnrollmentRepository
{
    internal const string StorageKey = "screen-controller.enrollment.v2";

    public async Task<DeviceEnrollment?> LoadAsync(CancellationToken cancellationToken = default)
    {
        var value = await secureStore.GetAsync(StorageKey, cancellationToken).ConfigureAwait(false);
        if (string.IsNullOrWhiteSpace(value))
        {
            return null;
        }

        try
        {
            var stored = JsonSerializer.Deserialize(value, EnrollmentJsonContext.Default.StoredEnrollment)
                ?? throw new JsonException("The secure enrollment value was empty.");
            var key = Convert.FromHexString(stored.KeyHex);
            if (key.Length != DeviceEnrollment.KeySize)
            {
                throw new JsonException("The secure enrollment key has an invalid length.");
            }

            return new DeviceEnrollment(stored.DeviceId, stored.DisplayName, key, stored.Psm);
        }
        catch (Exception exception) when (exception is JsonException or FormatException)
        {
            logger.LogWarning(exception, "Discarding an invalid secure enrollment value");
            await secureStore.RemoveAsync(StorageKey, cancellationToken).ConfigureAwait(false);
            return null;
        }
    }

    public Task SaveAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(enrollment);
        if (enrollment.Key.Length != DeviceEnrollment.KeySize)
        {
            throw new ArgumentException("Enrollment keys must contain exactly 32 bytes.", nameof(enrollment));
        }

        var value = JsonSerializer.Serialize(
            new StoredEnrollment(
                enrollment.DeviceId,
                enrollment.DisplayName,
                Convert.ToHexString(enrollment.Key),
                enrollment.Psm),
            EnrollmentJsonContext.Default.StoredEnrollment);
        return secureStore.SetAsync(StorageKey, value, cancellationToken);
    }

    public Task RemoveAsync(CancellationToken cancellationToken = default) =>
        secureStore.RemoveAsync(StorageKey, cancellationToken);
}

internal sealed record StoredEnrollment(string DeviceId, string DisplayName, string KeyHex, int Psm);

[JsonSerializable(typeof(StoredEnrollment))]
internal sealed partial class EnrollmentJsonContext : JsonSerializerContext;
