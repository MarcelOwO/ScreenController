using System.Security.Cryptography;
using System.Text;
using ScreenController.Domain;
using ScreenController.Protocol;
using Xunit;

namespace ScreenController.Protocol.Tests;

public sealed class PacketCodecTests
{
    [Fact]
    public void CommandRoundTrips()
    {
        var encoded = PacketCodec.Encode(
            new ProtocolPacket(PacketType.Command, "SetBrightness:42", ReadOnlyMemory<byte>.Empty));

        Assert.True(PacketCodec.TryDecode(encoded, out var decoded, out var consumed));
        Assert.Equal(encoded.Length, consumed);
        Assert.Equal(PacketType.Command, decoded!.Type);
        Assert.Equal("SetBrightness:42", decoded.Name);
        Assert.Empty(decoded.Payload.ToArray());
    }

    [Fact]
    public void PayloadRoundTripsWithNetworkByteOrderAndCrc()
    {
        var payload = Encoding.UTF8.GetBytes("hello over bluetooth");
        var encoded = PacketCodec.Encode(new ProtocolPacket(PacketType.Files, "files", payload));

        Assert.Equal(0xBE, encoded[0]);
        Assert.Equal(0xEF, encoded[1]);
        Assert.True(PacketCodec.TryDecode(encoded, out var decoded, out _));
        Assert.Equal(payload, decoded!.Payload.ToArray());
    }

    [Fact]
    public void AuthenticationMatchesKnownHmac()
    {
        var key = Enumerable.Range(0, 32).Select(value => (byte)value).ToArray();
        var nonce = Enumerable.Range(32, 32).Select(value => (byte)value).ToArray();
        var context = "screen-controller/auth/v2"u8.ToArray();
        byte[] message = [.. context, .. nonce];
        var expected = HMACSHA256.HashData(key, message);

        Assert.Equal(expected, Authentication.ComputeResponse(key, nonce));
    }

    [Fact]
    public void EnrollmentRoundTrips()
    {
        var enrollment = new DeviceEnrollment(
            "device-1", "Living Room", Enumerable.Repeat((byte)0xA5, 32).ToArray());

        var parsed = DeviceEnrollment.Parse(enrollment.ToEnrollmentUri());
        Assert.Equal(enrollment.DeviceId, parsed.DeviceId);
        Assert.Equal(enrollment.DisplayName, parsed.DisplayName);
        Assert.Equal(enrollment.Key, parsed.Key);
        Assert.Equal(enrollment.Psm, parsed.Psm);
    }
}
