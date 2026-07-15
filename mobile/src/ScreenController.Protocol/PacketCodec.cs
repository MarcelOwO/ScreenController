using System.Buffers.Binary;
using System.Text;

namespace ScreenController.Protocol;

public static class PacketCodec
{
    private const int HeaderSize = 7;
    private const int PayloadHeaderSize = 8;

    public static byte[] Encode(ProtocolPacket packet)
    {
        var name = Encoding.UTF8.GetBytes(packet.Name);
        if (name.Length > ProtocolConstants.MaxNameBytes)
        {
            throw new ProtocolException("Packet name exceeds 256 UTF-8 bytes.");
        }

        if (packet.Payload.Length > ProtocolConstants.MaxCompressedBytes)
        {
            throw new ProtocolException("Packet payload exceeds the 64 MiB transport limit.");
        }

        var payloadHeaderSize = packet.HasPayload ? PayloadHeaderSize : 0;
        var result = new byte[HeaderSize + name.Length + payloadHeaderSize + packet.Payload.Length];
        BinaryPrimitives.WriteUInt16BigEndian(result, ProtocolConstants.Magic);
        result[2] = (byte)packet.Type;
        BinaryPrimitives.WriteUInt32BigEndian(result.AsSpan(3), checked((uint)name.Length));
        name.CopyTo(result.AsSpan(HeaderSize));

        if (!packet.HasPayload)
        {
            return result;
        }

        var payloadOffset = HeaderSize + name.Length;
        BinaryPrimitives.WriteUInt32BigEndian(
            result.AsSpan(payloadOffset), checked((uint)packet.Payload.Length));
        BinaryPrimitives.WriteUInt32BigEndian(
            result.AsSpan(payloadOffset + 4), Crc32.Compute(packet.Payload.Span));
        packet.Payload.Span.CopyTo(result.AsSpan(payloadOffset + PayloadHeaderSize));
        return result;
    }

    public static bool TryDecode(ReadOnlySpan<byte> data, out ProtocolPacket? packet, out int consumed)
    {
        packet = null;
        consumed = 0;
        if (data.Length < HeaderSize)
        {
            return false;
        }

        if (BinaryPrimitives.ReadUInt16BigEndian(data) != ProtocolConstants.Magic)
        {
            throw new ProtocolException("Invalid packet magic.");
        }

        var type = (PacketType)data[2];
        var nameLength = BinaryPrimitives.ReadUInt32BigEndian(data[3..]);
        if (nameLength > ProtocolConstants.MaxNameBytes)
        {
            throw new ProtocolException("Packet name exceeds the protocol limit.");
        }

        var afterName = checked(HeaderSize + (int)nameLength);
        if (data.Length < afterName)
        {
            return false;
        }

        var name = Encoding.UTF8.GetString(data.Slice(HeaderSize, (int)nameLength));
        if ((byte)type < 0x80)
        {
            packet = new ProtocolPacket(type, name, ReadOnlyMemory<byte>.Empty);
            consumed = afterName;
            return true;
        }

        if (data.Length < afterName + PayloadHeaderSize)
        {
            return false;
        }

        var payloadLength = BinaryPrimitives.ReadUInt32BigEndian(data[afterName..]);
        if (payloadLength > ProtocolConstants.MaxCompressedBytes)
        {
            throw new ProtocolException("Packet payload exceeds the protocol limit.");
        }

        var packetLength = checked(afterName + PayloadHeaderSize + (int)payloadLength);
        if (data.Length < packetLength)
        {
            return false;
        }

        var expectedCrc = BinaryPrimitives.ReadUInt32BigEndian(data[(afterName + 4)..]);
        var payload = data.Slice(afterName + PayloadHeaderSize, (int)payloadLength).ToArray();
        if (Crc32.Compute(payload) != expectedCrc)
        {
            throw new ProtocolException("Packet CRC does not match its payload.");
        }

        packet = new ProtocolPacket(type, name, payload);
        consumed = packetLength;
        return true;
    }
}
