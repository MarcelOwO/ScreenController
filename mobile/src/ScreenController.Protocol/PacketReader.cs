namespace ScreenController.Protocol;

internal sealed class PacketReader
{
    private readonly IControllerTransport transport;
    private byte[] buffered = new byte[64 * 1024];
    private int count;

    public PacketReader(IControllerTransport transport) => this.transport = transport;

    public async Task<ProtocolPacket> ReadAsync(CancellationToken cancellationToken)
    {
        while (true)
        {
            if (PacketCodec.TryDecode(buffered.AsSpan(0, count), out var packet, out var consumed))
            {
                Buffer.BlockCopy(buffered, consumed, buffered, 0, count - consumed);
                count -= consumed;
                return packet!;
            }

            EnsureCapacity(count + 64 * 1024);
            var read = await transport.ReadAsync(buffered.AsMemory(count), cancellationToken)
                .ConfigureAwait(false);
            if (read == 0)
            {
                throw new EndOfStreamException("The screen controller closed the Bluetooth channel.");
            }

            count += read;
        }
    }

    private void EnsureCapacity(int required)
    {
        if (required <= buffered.Length)
        {
            return;
        }

        var maximum = ProtocolConstants.MaxCompressedBytes + ProtocolConstants.MaxNameBytes + 15;
        if (required > maximum)
        {
            throw new ProtocolException("Receive buffer exceeds the protocol limit.");
        }

        Array.Resize(ref buffered, Math.Min(maximum, Math.Max(required, buffered.Length * 2)));
    }
}
