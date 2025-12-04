#pragma once

#include <QVulkanWindow>
#include <QWidget>

namespace PelicanStudio {

class PlayerScreen : public QWidget {
    Q_OBJECT
  public:
    PlayerScreen();

    Q_INVOKABLE VkSurfaceKHR getSurface(VkInstance vk_instance);
};

} // namespace PelicanStudio
