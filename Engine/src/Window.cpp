#include "Window.h"

#include "Core.h"
#include "GLFW/glfw3.h"

namespace Ash {

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->framebufferResized = true;
}

static void mouse_cursor_callback(GLFWwindow *window, double xpos,
                                  double ypos) {}

Window::Window() {}

Window::~Window() {}

void Window::init() { ASH_ASSERT(glfwInit(), "Failed to initialize GLFW"); }

void Window::cleanup() {
  ASH_INFO("Terminating GLFW");
  glfwTerminate();
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window); }

void Window::swapBuffers() const { glfwSwapBuffers(window); }

void Window::pollEvents() const { glfwPollEvents(); }

GLFWwindow *Window::get() const { return window; }

std::pair<int, int> Window::getWindowSize() const {
  std::pair<int, int> pos;
  glfwGetWindowSize(window, &pos.first, &pos.second);
  return pos;
}

std::pair<double, double> Window::getCursorPos() const {
  std::pair<double, double> pos;
  glfwGetCursorPos(window, &pos.first, &pos.second);
  return pos;
}

EventStatus Window::getMouseButton(MouseButton button) const {
  return EventStatus(glfwGetMouseButton(window, button));
}

EventStatus Window::getKey(Key key) const {
  return EventStatus(glfwGetKey(window, key));
}

void Window::setCursorPos(double x, double y) const {
  glfwSetCursorPos(window, x, y);
}

void Window::disableCursor() const {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::enableCursor() const {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}


std::shared_ptr<Window> Window::create(const WindowProperties &properties) {
  std::shared_ptr<Window> ret = std::make_shared<Window>();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  ret->window = glfwCreateWindow(properties.width, properties.height,
                                 properties.title.c_str(), nullptr, nullptr);
  ASH_ASSERT(ret, "Failed to create window");
  glfwSetFramebufferSizeCallback(ret->get(), framebufferResizeCallback);
  glfwSetCursorPosCallback(ret->get(), mouse_cursor_callback);
  glfwSetWindowUserPointer(ret->get(), ret.get());
  return ret;
}

void Window::destroy() {
  ASH_INFO("Destroying window...");
  glfwDestroyWindow(window);
}

} // namespace Ash
