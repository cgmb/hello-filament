#include "native_window.h"
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

void* getNativeWindow(GLFWwindow* window) {
  return (void*)glfwGetX11Window(window);
}
