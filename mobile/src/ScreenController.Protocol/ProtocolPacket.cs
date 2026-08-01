namespace ScreenController.Protocol;

public sealed record ProtocolPacket(PacketType Type, string Name, ReadOnlyMemory<byte> Payload)
{
    public bool HasPayload => (byte)Type >= 0x80;
}
