#include "window.hpp"

namespace Pelican {

Window::Window() {}

Window::~Window() {}

void Window::setScreen(AbstractScreen *new_screen) { screen = new_screen; }

bool Window::process() { return screen->process(); }
vk::SurfaceKHR Window::getVulkanSurface(vk::Instance instance) { return screen->getVulkanSurface(instance); }
std::vector<const char *> Window::getRequiredVulkanInstanceExts() { return screen->getRequiredVulkanExtensions(); }

} // namespace Pelican
