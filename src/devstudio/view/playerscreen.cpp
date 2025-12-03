#include "playerscreen.hpp"
#include <QBoxLayout>
#include <QVulkanWindow>

namespace PelicanStudio {

PlayerScreen::PlayerScreen() {}

VkSurfaceKHR PlayerScreen::getSurface(VkInstance vk_instance) {
    QVulkanInstance instance;
    instance.setVkInstance(vk_instance);
    instance.create();
    QWindow *window = new QWindow;
    window->setSurfaceType(QSurface::VulkanSurface);
    window->setVulkanInstance(&instance);
    QWidget *wrapper = QWidget::createWindowContainer(window, this);
    VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(window);

    QLayout *layout = new QVBoxLayout();
    layout->addWidget(wrapper);
    setLayout(layout);

    return surface;
}

} // namespace PelicanStudio