#pragma once

#include <vulkan/vulkan.hpp>

#include "../container.hpp"
#include "../userpublic/abstractscreen.hpp"

namespace Pelican {

DECLARE_MODULE(Window) {
    AbstractScreen *screen;

  public:
    Window();
    ~Window();

    void setScreen(AbstractScreen * _screen);

    vk::SurfaceKHR getVulkanSurface(vk::Instance instance);
    std::vector<const char *> getRequiredVulkanInstanceExts();

    // false to close
    bool process();
};

} // namespace Pelican
