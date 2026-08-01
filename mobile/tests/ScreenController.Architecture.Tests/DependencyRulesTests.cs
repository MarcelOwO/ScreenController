using Xunit;

namespace ScreenController.Architecture.Tests;

public sealed class DependencyRulesTests
{
    [Fact]
    public void DomainHasNoProjectDependencies()
    {
        var dependencies = typeof(Domain.DeviceEnrollment).Assembly.GetReferencedAssemblies();

        Assert.DoesNotContain(dependencies, assembly => assembly.Name?.StartsWith("ScreenController.", StringComparison.Ordinal) == true);
    }

    [Fact]
    public void ApplicationDoesNotReferenceOuterLayers()
    {
        var dependencies = typeof(Application.EnrollmentService).Assembly.GetReferencedAssemblies();

        Assert.DoesNotContain(dependencies, assembly => assembly.Name is
            "ScreenController.App" or
            "ScreenController.Infrastructure" or
            "ScreenController.Presentation" or
            "ScreenController.Protocol");
    }

    [Fact]
    public void PresentationDoesNotReferenceMauiOrInfrastructure()
    {
        var dependencies = typeof(Presentation.ControllerViewModel).Assembly.GetReferencedAssemblies();

        Assert.DoesNotContain(dependencies, assembly =>
            assembly.Name?.StartsWith("Microsoft.Maui", StringComparison.Ordinal) == true ||
            assembly.Name is "ScreenController.App" or "ScreenController.Infrastructure" or "ScreenController.Protocol");
    }
}
