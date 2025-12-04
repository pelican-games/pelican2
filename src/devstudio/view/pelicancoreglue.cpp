#include "pelicancoreglue.hpp"

Q_DECLARE_OPAQUE_POINTER(VkInstance);
Q_DECLARE_OPAQUE_POINTER(VkSurfaceKHR);

namespace PelicanStudio {

PelicanCoreGlue::PelicanCoreGlue(PlayerScreen *_screen) : screen{_screen} {
    qRegisterMetaType<VkSurfaceKHR>("VkSurfaceKHR");
    qRegisterMetaType<VkInstance>("VkInstance");
}

VkSurfaceKHR PelicanCoreGlue::getVulkanSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    QMetaObject::invokeMethod(screen, "getSurface", Qt::BlockingQueuedConnection, Q_RETURN_ARG(VkSurfaceKHR, surface),
                              Q_ARG(VkInstance, instance));
    return surface;
}
std::vector<const char *> PelicanCoreGlue::getRequiredVulkanExtensions() { return {}; }
bool PelicanCoreGlue::process() { return true; }

} // namespace PelicanStudio
