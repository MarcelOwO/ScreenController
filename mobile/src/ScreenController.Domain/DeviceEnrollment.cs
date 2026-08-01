using System.Collections.Specialized;
using System.Globalization;
using System.Web;

namespace ScreenController.Domain;

public sealed record DeviceEnrollment(
    string DeviceId,
    string DisplayName,
    byte[] Key,
    int Psm = 0x0081)
{
    public const int KeySize = 32;

    public static DeviceEnrollment Parse(string value)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(value);

        if (!Uri.TryCreate(value.Trim(), UriKind.Absolute, out var uri) ||
            !string.Equals(uri.Scheme, "screencontroller", StringComparison.Ordinal) ||
            !string.Equals(uri.Host, "enroll", StringComparison.Ordinal))
        {
            throw new FormatException("Enrollment must be a screencontroller://enroll URI.");
        }

        NameValueCollection query = HttpUtility.ParseQueryString(uri.Query);
        var keyText = query["key"] ?? throw new FormatException("Enrollment key is missing.");
        if (keyText.Length != KeySize * 2)
        {
            throw new FormatException("Enrollment key must contain 64 hexadecimal characters.");
        }

        byte[] key;
        try
        {
            key = Convert.FromHexString(keyText);
        }
        catch (FormatException exception)
        {
            throw new FormatException("Enrollment key is not valid hexadecimal.", exception);
        }

        var psm = int.TryParse(query["psm"], NumberStyles.None, CultureInfo.InvariantCulture, out var parsedPsm)
            ? parsedPsm
            : 0x0081;
        if (psm is <= 0 or > ushort.MaxValue)
        {
            throw new FormatException("Enrollment PSM is outside the valid range.");
        }

        return new DeviceEnrollment(
            query["id"] ?? string.Empty,
            query["name"] ?? "ScreenController",
            key,
            psm);
    }

    public string ToEnrollmentUri()
    {
        if (Key.Length != KeySize)
        {
            throw new InvalidOperationException("Enrollment keys must contain exactly 32 bytes.");
        }

        var query = HttpUtility.ParseQueryString(string.Empty);
        query["v"] = "2";
        query["id"] = DeviceId;
        query["name"] = DisplayName;
        query["key"] = Convert.ToHexString(Key).ToLowerInvariant();
        query["psm"] = Psm.ToString(CultureInfo.InvariantCulture);
        return $"screencontroller://enroll?{query}";
    }
}
