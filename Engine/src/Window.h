#pragma once

#include <GLFW/glfw3.h>

#include <memory>
#include <string>

namespace Ash {

enum MouseButton {
  MOUSE_BUTTON_1 = GLFW_MOUSE_BUTTON_1,
  MOUSE_BUTTON_2 = GLFW_MOUSE_BUTTON_2,
  MOUSE_BUTTON_3 = GLFW_MOUSE_BUTTON_3,
};

enum Key {
  KEY_W = GLFW_KEY_W,
  KEY_S = GLFW_KEY_S,
  KEY_D = GLFW_KEY_D,
  KEY_A = GLFW_KEY_A,
  KEY_LSHIFT = GLFW_KEY_LEFT_SHIFT,
  KEY_SPACE = GLFW_KEY_SPACE,
};

enum EventStatus { PRESSED = GLFW_PRESS, RELEASED = GLFW_RELEASE };

struct WindowProperties {
  std::string title;
  uint32_t width, height;

  WindowProperties(const std::string &title, uint32_t width, uint32_t height)
      : title(title), width(width), height(height) {}
};

class Window {
public:
  Window();
  ~Window();

  static void init();
  static void cleanup();

  bool shouldClose() const;
  void swapBuffers() const;
  void pollEvents() const;
  GLFWwindow *get() const;

  std::pair<int, int> getWindowSize() const;
  std::pair<double, double> getCursorPos() const;
  EventStatus getMouseButton(MouseButton button) const;
  EventStatus getKey(Key key) const;
  void setCursorPos(double x, double y) const;
  void disableCursor() const;
  void enableCursor() const;

  static std::shared_ptr<Window> create(const WindowProperties &properties);
  void destroy();

  bool framebufferResized = false;

private:
  GLFWwindow *window;
};

} // namespace Ash
