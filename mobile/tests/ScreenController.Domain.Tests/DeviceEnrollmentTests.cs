using ScreenController.Domain;
using Xunit;

namespace ScreenController.Domain.Tests;

public sealed class DeviceEnrollmentTests
{
    [Fact]
    public void EnrollmentRoundTripsAllValues()
    {
        var expected = new DeviceEnrollment(
            "pi:living-room",
            "Living Room Display",
            Enumerable.Range(0, 32).Select(value => (byte)value).ToArray(),
            0x0083);

        var actual = DeviceEnrollment.Parse(expected.ToEnrollmentUri());

        Assert.Equal(expected.DeviceId, actual.DeviceId);
        Assert.Equal(expected.DisplayName, actual.DisplayName);
        Assert.Equal(expected.Key, actual.Key);
        Assert.Equal(expected.Psm, actual.Psm);
    }

    [Theory]
    [InlineData("")]
    [InlineData("https://example.com/enroll?key=00")]
    [InlineData("screencontroller://other?key=00")]
    [InlineData("screencontroller://enroll?key=not-hex")]
    [InlineData("screencontroller://enroll?key=0000000000000000000000000000000000000000000000000000000000000000&psm=0")]
    public void InvalidEnrollmentIsRejected(string value) =>
        Assert.NotNull(Record.Exception(() => DeviceEnrollment.Parse(value)));

    [Theory]
    [InlineData(0, 0)]
    [InlineData(50, 0.5)]
    [InlineData(150, 1)]
    public void TransferProgressIsClamped(long sent, double fraction) =>
        Assert.Equal(fraction, new TransferProgress(sent, 100).Fraction);
}
