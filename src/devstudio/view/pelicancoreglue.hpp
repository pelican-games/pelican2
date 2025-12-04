#pragma once

#include "playerscreen.hpp"
#include <QObject>
#include <pelican_core.hpp>

namespace PelicanStudio {

class PelicanCoreGlue : public QObject, public Pelican::AbstractScreen {
    Q_OBJECT

    PlayerScreen *screen;

  public:
    PelicanCoreGlue(PlayerScreen *screen);

    VkSurfaceKHR getVulkanSurface(VkInstance instance) override;
    std::vector<const char *> getRequiredVulkanExtensions() override;
    bool process() override;
};

} // namespace PelicanStudio
