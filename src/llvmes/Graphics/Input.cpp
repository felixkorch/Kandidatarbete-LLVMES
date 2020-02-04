#include "llvmes/Graphics/Input.h"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include "GLFW/glfw3.h"
namespace llvmes {
	namespace graphics {
		bool Input::isKeyPressed(void* window, int key)
		{
			return glfwGetKey((GLFWwindow*)window, key);
		}
	}
}