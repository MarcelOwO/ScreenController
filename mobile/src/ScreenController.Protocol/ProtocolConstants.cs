namespace ScreenController.Protocol;

public static class ProtocolConstants
{
    public const ushort Magic = 0xBEEF;
    public const int LePsm = 0x0081;
    public const int MaxNameBytes = 256;
    public const int MaxCompressedBytes = 64 * 1024 * 1024;
    public const int MaxFileBytes = 128 * 1024 * 1024;

    public static readonly Guid ServiceUuid = new("8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a001");
    public static readonly Guid GattWriteUuid = new("8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a002");
    public static readonly Guid GattNotifyUuid = new("8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a003");

    public static ReadOnlySpan<byte> AuthenticationContext => "screen-controller/auth/v2"u8;
}

public enum PacketType : byte
{
    Command = 0x01,
    AuthenticationResponse = 0x80,
    FileTransfer = 0x81,
    AuthenticationChallenge = 0xC0,
    Files = 0xC1,
    Acknowledgement = 0xC2,
    Status = 0xC3,
    Error = 0xCF,
}
