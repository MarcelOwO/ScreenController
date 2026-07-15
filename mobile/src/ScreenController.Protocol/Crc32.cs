namespace ScreenController.Protocol;

internal static class Crc32
{
    public static uint Compute(ReadOnlySpan<byte> bytes)
    {
        var crc = uint.MaxValue;
        foreach (var value in bytes)
        {
            crc ^= value;
            for (var bit = 0; bit < 8; bit++)
            {
                crc = (crc >> 1) ^ (0xEDB88320U & (uint)-(int)(crc & 1));
            }
        }

        return ~crc;
    }
}
