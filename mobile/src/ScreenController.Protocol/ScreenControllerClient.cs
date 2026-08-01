using System.Text;
using System.Text.Json;
using System.Threading.Channels;
using Microsoft.Extensions.Logging;
using ScreenController.Application.Abstractions;
using ScreenController.Domain;
using ZstdSharp;

namespace ScreenController.Protocol;

public sealed class ScreenControllerClient : IDisplayController
{
    private static readonly TimeSpan ResponseTimeout = TimeSpan.FromSeconds(15);
    private readonly IControllerTransportFactory transportFactory;
    private readonly ILogger<ScreenControllerClient> logger;
    private readonly SemaphoreSlim operationLock = new(1, 1);
    private Channel<ProtocolPacket> packets = CreatePacketChannel();
    private IControllerTransport? transport;
    private PacketReader? reader;
    private CancellationTokenSource? connectionCancellation;
    private Task? receiveTask;

    public ScreenControllerClient(
        IControllerTransportFactory transportFactory,
        ILogger<ScreenControllerClient> logger)
    {
        this.transportFactory = transportFactory;
        this.logger = logger;
    }

    public bool IsConnected => transport?.IsConnected == true && receiveTask is { IsCompleted: false };

    public event EventHandler<bool>? ConnectionChanged;

    public IAsyncEnumerable<DeviceCandidate> ScanAsync(CancellationToken cancellationToken) =>
        transportFactory.ScanAsync(cancellationToken);

    public async Task ConnectAsync(DeviceEnrollment enrollment, CancellationToken cancellationToken)
    {
        using var scope = logger.BeginScope(new Dictionary<string, object?>
        {
            ["DeviceId"] = enrollment.DeviceId,
        });
        logger.LogInformation("Opening Bluetooth transport");
        await DisconnectAsync().ConfigureAwait(false);
        packets = CreatePacketChannel();
        transport = await transportFactory.ConnectAsync(enrollment, cancellationToken)
            .ConfigureAwait(false);
        reader = new PacketReader(transport);

        try
        {
            var challenge = await reader.ReadAsync(cancellationToken).ConfigureAwait(false);
            if (challenge.Type != PacketType.AuthenticationChallenge ||
                challenge.Name != "auth-v2" || challenge.Payload.Length != 32)
            {
                throw new ProtocolException("The controller did not send a valid authentication challenge.");
            }

            var response = Authentication.ComputeResponse(enrollment.Key, challenge.Payload.Span);
            await WritePacketAsync(
                new ProtocolPacket(PacketType.AuthenticationResponse, "auth-v2", response),
                null,
                cancellationToken).ConfigureAwait(false);

            connectionCancellation = new CancellationTokenSource();
            receiveTask = ReceiveLoopAsync(connectionCancellation.Token);
            logger.LogInformation("Bluetooth transport authenticated");
            ConnectionChanged?.Invoke(this, true);
        }
        catch (Exception exception)
        {
            logger.LogError(exception, "Bluetooth connection or authentication failed");
            await DisconnectAsync().ConfigureAwait(false);
            throw;
        }
    }

    public async Task<IReadOnlyList<string>> GetFilesAsync(CancellationToken cancellationToken) =>
        await ExecuteAsync(
            "GetFiles",
            PacketType.Files,
            packet => Encoding.UTF8.GetString(packet.Payload.Span)
                .Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries),
            cancellationToken).ConfigureAwait(false);

    public async Task<DisplayStatus> GetStatusAsync(CancellationToken cancellationToken) =>
        await ExecuteAsync(
            "GetStatus",
            PacketType.Status,
            packet => JsonSerializer.Deserialize<DisplayStatus>(packet.Payload.Span,
                          new JsonSerializerOptions { PropertyNameCaseInsensitive = true }) ??
                      throw new ProtocolException("The controller returned an empty status response."),
            cancellationToken).ConfigureAwait(false);

    public Task RotateAsync(CancellationToken cancellationToken) =>
        ExecuteAcknowledgedAsync("Rotate", "Rotate", cancellationToken);

    public Task SelectAsync(string fileName, CancellationToken cancellationToken) =>
        ExecuteAcknowledgedAsync($"Select:{fileName}", "Select", cancellationToken);

    public Task DeleteAsync(string fileName, CancellationToken cancellationToken) =>
        ExecuteAcknowledgedAsync($"Delete:{fileName}", "Delete", cancellationToken);

    public Task SetBrightnessAsync(int brightness, CancellationToken cancellationToken)
    {
        if (brightness is < 0 or > 100)
        {
            throw new ArgumentOutOfRangeException(nameof(brightness));
        }

        return ExecuteAcknowledgedAsync(
            $"SetBrightness:{brightness}", "SetBrightness", cancellationToken);
    }

    public Task SetScreenEnabledAsync(bool enabled, CancellationToken cancellationToken) =>
        ExecuteAcknowledgedAsync(enabled ? "ScreenOn" : "ScreenOff", enabled ? "ScreenOn" : "ScreenOff",
            cancellationToken);

    public async Task UploadAsync(
        string fileName,
        ReadOnlyMemory<byte> file,
        IProgress<TransferProgress>? progress,
        CancellationToken cancellationToken)
    {
        logger.LogInformation("Preparing upload {FileName} with {FileBytes} bytes", Path.GetFileName(fileName), file.Length);
        if (file.Length > ProtocolConstants.MaxFileBytes)
        {
            throw new ProtocolException("Files larger than 128 MiB are not supported.");
        }

        byte[] compressed;
        using (var compressor = new Compressor(1))
        {
            compressed = compressor.Wrap(file.Span).ToArray();
        }

        if (compressed.Length > ProtocolConstants.MaxCompressedBytes)
        {
            throw new ProtocolException("Compressed upload exceeds the 64 MiB transport limit.");
        }

        await operationLock.WaitAsync(cancellationToken).ConfigureAwait(false);
        try
        {
            await WritePacketAsync(
                new ProtocolPacket(PacketType.FileTransfer, Path.GetFileName(fileName), compressed),
                progress,
                cancellationToken).ConfigureAwait(false);
            var response = await ReadResponseAsync(cancellationToken).ConfigureAwait(false);
            EnsureResponse(response, PacketType.Acknowledgement, "Upload");
            logger.LogInformation("Upload {FileName} completed", Path.GetFileName(fileName));
        }
        finally
        {
            operationLock.Release();
        }
    }

    public async Task DisconnectAsync()
    {
        var wasConnected = transport is not null;
        connectionCancellation?.Cancel();
        if (transport is not null)
        {
            await transport.DisposeAsync().ConfigureAwait(false);
        }

        transport = null;
        reader = null;
        connectionCancellation?.Dispose();
        connectionCancellation = null;
        receiveTask = null;
        while (packets.Reader.TryRead(out _)) { }
        if (wasConnected)
        {
            logger.LogInformation("Bluetooth transport disconnected");
        }
        ConnectionChanged?.Invoke(this, false);
    }

    public async ValueTask DisposeAsync()
    {
        await DisconnectAsync().ConfigureAwait(false);
        operationLock.Dispose();
    }

    private Task ExecuteAcknowledgedAsync(string command, string acknowledgement, CancellationToken token) =>
        ExecuteAsync(
            command,
            PacketType.Acknowledgement,
            packet =>
            {
                if (packet.Name != acknowledgement)
                {
                    throw new ProtocolException($"Expected {acknowledgement} acknowledgement, got {packet.Name}.");
                }

                return true;
            },
            token);

    private async Task<T> ExecuteAsync<T>(
        string command,
        PacketType expectedType,
        Func<ProtocolPacket, T> convert,
        CancellationToken cancellationToken)
    {
        await operationLock.WaitAsync(cancellationToken).ConfigureAwait(false);
        try
        {
            await WritePacketAsync(
                new ProtocolPacket(PacketType.Command, command, ReadOnlyMemory<byte>.Empty),
                null,
                cancellationToken).ConfigureAwait(false);
            var response = await ReadResponseAsync(cancellationToken).ConfigureAwait(false);
            EnsureResponse(response, expectedType, null);
            return convert(response);
        }
        finally
        {
            operationLock.Release();
        }
    }

    private async Task WritePacketAsync(
        ProtocolPacket packet,
        IProgress<TransferProgress>? progress,
        CancellationToken cancellationToken)
    {
        var activeTransport = transport ?? throw new InvalidOperationException("Not connected.");
        await activeTransport.WriteAsync(PacketCodec.Encode(packet), progress, cancellationToken)
            .ConfigureAwait(false);
    }

    private async Task<ProtocolPacket> ReadResponseAsync(CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(ResponseTimeout);
        try
        {
            return await packets.Reader.ReadAsync(timeout.Token).ConfigureAwait(false);
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            throw new TimeoutException("The screen controller did not respond within 15 seconds.");
        }
    }

    private async Task ReceiveLoopAsync(CancellationToken cancellationToken)
    {
        try
        {
            while (!cancellationToken.IsCancellationRequested)
            {
                var packet = await reader!.ReadAsync(cancellationToken).ConfigureAwait(false);
                await packets.Writer.WriteAsync(packet, cancellationToken).ConfigureAwait(false);
            }
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested) { }
        catch (Exception exception)
        {
            logger.LogError(exception, "Bluetooth receive loop terminated unexpectedly");
            packets.Writer.TryComplete(exception);
            ConnectionChanged?.Invoke(this, false);
        }
    }

    private static void EnsureResponse(
        ProtocolPacket packet,
        PacketType expectedType,
        string? expectedName)
    {
        if (packet.Type == PacketType.Error)
        {
            throw new ProtocolException(Encoding.UTF8.GetString(packet.Payload.Span));
        }

        if (packet.Type != expectedType || (expectedName is not null && packet.Name != expectedName))
        {
            throw new ProtocolException(
                $"Unexpected response {packet.Type}/{packet.Name}; expected {expectedType}/{expectedName ?? "*"}.");
        }
    }

    private static Channel<ProtocolPacket> CreatePacketChannel() =>
        Channel.CreateUnbounded<ProtocolPacket>(
            new UnboundedChannelOptions { SingleReader = true, SingleWriter = true });
}
