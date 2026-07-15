using System.Text.Json;
using ScreenController.Protocol;

namespace ScreenController.App.Services;

public sealed class EnrollmentStore
{
    private const string StorageKey = "screen-controller.enrollment.v2";
    public async Task<DeviceEnrollment?> LoadAsync()
    {
        var value = await SecureStorage.Default.GetAsync(StorageKey);
        var stored = string.IsNullOrWhiteSpace(value) ? null : JsonSerializer.Deserialize<StoredEnrollment>(value);
        return stored is null ? null : new(stored.DeviceId, stored.DisplayName, Convert.FromHexString(stored.KeyHex), stored.Psm);
    }

    public Task SaveAsync(DeviceEnrollment value) => SecureStorage.Default.SetAsync(StorageKey,
        JsonSerializer.Serialize(new StoredEnrollment(value.DeviceId, value.DisplayName, Convert.ToHexString(value.Key), value.Psm)));
    public bool Remove() => SecureStorage.Default.Remove(StorageKey);
    private sealed record StoredEnrollment(string DeviceId, string DisplayName, string KeyHex, int Psm);
}
