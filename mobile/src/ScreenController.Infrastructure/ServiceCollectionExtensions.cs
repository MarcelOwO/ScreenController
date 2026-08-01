using Microsoft.Extensions.DependencyInjection;
using ScreenController.Application.Abstractions;

namespace ScreenController.Infrastructure;

public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddScreenControllerInfrastructure(this IServiceCollection services)
    {
        ArgumentNullException.ThrowIfNull(services);
        services.AddSingleton<IEnrollmentRepository, SecureEnrollmentRepository>();
        return services;
    }
}
