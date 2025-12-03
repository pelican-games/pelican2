#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <abstractscreen.hpp>
#include <iterator>
#include <pelican_core.hpp>
#include <stdexcept>

class MainWindow : public Pelican::AbstractScreen {
    GLFWwindow *window;

    VkInstance instance;
    VkSurfaceKHR surface;

  public:
    MainWindow() {
        const int window_size_width = 1280;
        const int window_size_height = 720;
        const std::string title = "AppTitle";
        const bool fullscreen = false;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(window_size_width, window_size_height, title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        // glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        //     if (action == GLFW_REPEAT)
        //         return;
        //     const auto thiz = static_cast<Window *>(glfwGetWindowUserPointer(window));

        //     // action: GLFW_PRESS == 1, GLFW_RELEASE == 0
        //     thiz->key_state.pressing.set(button_id_offset_keyboard + key, action & 1);
        // });
        // glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
        //     const auto thiz = static_cast<Window *>(glfwGetWindowUserPointer(window));

        //     // action: GLFW_PRESS == 1, GLFW_RELEASE == 0
        //     thiz->key_state.pressing.set(button_id_offset_mouse_button + button, action & 1);
        // });
        // glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
        //     const auto thiz = static_cast<Window *>(glfwGetWindowUserPointer(window));

        //     thiz->cursor_x = static_cast<float>(xpos);
        //     thiz->cursor_y = static_cast<float>(ypos);
        // });
    }

    VkSurfaceKHR getVulkanSurface(VkInstance instance) override {
        VkSurfaceKHR surface;
        if (auto result = glfwCreateWindowSurface(instance, window, nullptr, &surface); result != VK_SUCCESS) {
            throw std::runtime_error("glfwCreateWindowSurface failed: code=" + result);
        }

        return surface;
    }

    std::vector<const char *> getRequiredVulkanExtensions() override {
        uint32_t count;
        const char **ext_names_raw = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char *> ext_names;
        std::copy(ext_names_raw, ext_names_raw + count, std::back_inserter(ext_names));

        return ext_names;
    }

    bool process() override {
        glfwPollEvents();
        return !glfwWindowShouldClose(window);
    }
};

int main() {
    MainWindow window;
    Pelican::PelicanCore pl;
    pl.setScreen(&window);
    pl.run();
    return 0;
}
