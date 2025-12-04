#include "playerscreen.hpp"
#include <QBoxLayout>
#include <QVulkanWindow>

namespace PelicanStudio {

PlayerScreen::PlayerScreen() {}

VkSurfaceKHR PlayerScreen::getSurface(VkInstance vk_instance) {
    QVulkanInstance instance;
    instance.setVkInstance(vk_instance);
    instance.create();
    QWindow *window = new QVulkanWindow;
    window->setSurfaceType(QSurface::VulkanSurface);
    window->setVulkanInstance(&instance);
    window->show();
    QWidget *wrapper = QWidget::createWindowContainer(window, this);

    QLayout *layout = new QVBoxLayout();
    layout->addWidget(wrapper);
    setLayout(layout);
    VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(window);

    return surface;
}

} // namespace PelicanStudio