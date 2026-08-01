using Microsoft.Extensions.DependencyInjection;

namespace ScreenController.Application;

public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddScreenControllerApplication(this IServiceCollection services)
    {
        ArgumentNullException.ThrowIfNull(services);
        services.AddSingleton<EnrollmentService>();
        return services;
    }
}
