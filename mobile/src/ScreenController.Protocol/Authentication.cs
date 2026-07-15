using System.Security.Cryptography;

namespace ScreenController.Protocol;

public static class Authentication
{
    public static byte[] ComputeResponse(ReadOnlySpan<byte> key, ReadOnlySpan<byte> nonce)
    {
        if (key.Length != 32)
        {
            throw new ArgumentException("The device key must contain 32 bytes.", nameof(key));
        }

        if (nonce.Length != 32)
        {
            throw new ArgumentException("The authentication nonce must contain 32 bytes.", nameof(nonce));
        }

        var message = new byte[ProtocolConstants.AuthenticationContext.Length + nonce.Length];
        ProtocolConstants.AuthenticationContext.CopyTo(message);
        nonce.CopyTo(message.AsSpan(ProtocolConstants.AuthenticationContext.Length));
        return HMACSHA256.HashData(key, message);
    }
}
