#pragma once

#include <abstractscreen.hpp>
#include <string>

namespace Pelican {

class PelicanCore {
    std::string settings_str;

  public:
    PelicanCore();
    PelicanCore(std::string settings);

    void setScreen(AbstractScreen *screen);
    void run();
};

} // namespace Pelican
