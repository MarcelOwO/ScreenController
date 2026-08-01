using Microsoft.Extensions.DependencyInjection;

namespace ScreenController.Presentation;

public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddScreenControllerPresentation(this IServiceCollection services)
    {
        ArgumentNullException.ThrowIfNull(services);
        services.AddSingleton<ControllerViewModel>();
        return services;
    }
}
