namespace ScreenController.Protocol;

public sealed record ProtocolPacket(PacketType Type, string Name, ReadOnlyMemory<byte> Payload)
{
    public bool HasPayload => (byte)Type >= 0x80;
}

public sealed record DisplayStatus(int Brightness, bool DisplayEnabled);

public sealed record DeviceCandidate(string Id, string Name, int SignalStrength);

public sealed record TransferProgress(long BytesSent, long TotalBytes)
{
    public double Fraction => TotalBytes <= 0 ? 0 : (double)BytesSent / TotalBytes;
}
