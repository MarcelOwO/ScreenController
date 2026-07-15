using System.Collections.Specialized;
using System.Web;

namespace ScreenController.Protocol;

public sealed record DeviceEnrollment(
    string DeviceId,
    string DisplayName,
    byte[] Key,
    int Psm = ProtocolConstants.LePsm)
{
    public static DeviceEnrollment Parse(string value)
    {
        if (!Uri.TryCreate(value.Trim(), UriKind.Absolute, out var uri) ||
            uri.Scheme != "screencontroller" || uri.Host != "enroll")
        {
            throw new FormatException("Enrollment must be a screencontroller://enroll URI.");
        }

        NameValueCollection query = HttpUtility.ParseQueryString(uri.Query);
        var keyText = query["key"] ?? throw new FormatException("Enrollment key is missing.");
        if (keyText.Length != 64)
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

        var id = query["id"] ?? string.Empty;
        var name = query["name"] ?? "ScreenController";
        var psm = int.TryParse(query["psm"], out var parsedPsm)
            ? parsedPsm
            : ProtocolConstants.LePsm;
        if (psm is <= 0 or > ushort.MaxValue)
        {
            throw new FormatException("Enrollment PSM is outside the valid range.");
        }

        return new DeviceEnrollment(id, name, key, psm);
    }

    public string ToEnrollmentUri()
    {
        var query = HttpUtility.ParseQueryString(string.Empty);
        query["v"] = "2";
        query["id"] = DeviceId;
        query["name"] = DisplayName;
        query["key"] = Convert.ToHexString(Key).ToLowerInvariant();
        query["psm"] = Psm.ToString(System.Globalization.CultureInfo.InvariantCulture);
        return $"screencontroller://enroll?{query}";
    }
}
