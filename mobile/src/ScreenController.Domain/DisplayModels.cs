namespace ScreenController.Domain;

public sealed record DisplayStatus(int Brightness, bool DisplayEnabled);

public sealed record DeviceCandidate(string Id, string Name, int SignalStrength);

public sealed record TransferProgress(long BytesSent, long TotalBytes)
{
    public double Fraction => TotalBytes <= 0
        ? 0
        : Math.Clamp((double)BytesSent / TotalBytes, 0, 1);
}
