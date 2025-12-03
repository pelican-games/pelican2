#include <vector>
#include <vulkan/vulkan.h>

namespace Pelican {

class AbstractScreen {
  public:
    virtual VkSurfaceKHR getVulkanSurface(VkInstance instance) = 0;
    virtual std::vector<const char *> getRequiredExtensions() = 0;
    virtual bool process() = 0;
};

} // namespace Pelican
