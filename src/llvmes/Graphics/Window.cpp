#include "llvmes/Graphics/Window.h"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include <string>
#include <iostream>

namespace llvmes {
	namespace graphics {

		/// Forward declarations of the callbacks.
		static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void windowResizedCallback(GLFWwindow* window, int width, int height);

		Window::Window(int width, int height, const std::string& title, bool resizable)
			: title(title)
			, width(width)
			, height(height)
			, resizable(resizable)
			, eventQueue()
		{
			if (!glfwInit()) {
				std::cout << "Failed to create window." << std::endl;
			}

			/// Necessary hints, specifically for mac/linux
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, 4);
			glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_FALSE : GL_TRUE);

			GLFWwindow* createdWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

			if (!createdWindow) {
				glfwTerminate();
				std::cout << "Failed to create window." << std::endl;
			}

			glfwMakeContextCurrent(createdWindow);

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
				std::cout << "Failed to initialize OpenGL" << std::endl;
			}
			std::cout << "Window successfully created" << std::endl;
			glfwWindow = createdWindow;
            
            glfwSetWindowUserPointer((GLFWwindow*)glfwWindow, this);

			/// Callback to set the viewport to match the new size of the window.
			glfwSetFramebufferSizeCallback((GLFWwindow*)glfwWindow, frameBufferSizeCallback);
			glfwSetKeyCallback((GLFWwindow*)glfwWindow, keyCallback);
			glfwSetWindowSizeCallback((GLFWwindow*)glfwWindow, windowResizedCallback);
		}

		bool Window::isOpen() const
		{
			return !glfwWindowShouldClose((GLFWwindow*)glfwWindow);
		}

		void Window::clear()
		{
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void Window::update()
		{
			glfwSwapBuffers((GLFWwindow*)glfwWindow);
			glfwPollEvents();
		}

		bool Window::pollEvent(Event& e)
		{
			if (eventQueue.empty())
				return false;
			e = eventQueue.front();
			eventQueue.pop();
			return true;
		}

		void Window::pushEvent(Event& e)
		{
			/// Clear if queue gets too large.
			if (eventQueue.size() > 200) {
				std::queue<Event> empty;
				std::swap(eventQueue, empty);
			}
			eventQueue.push(e);
		}

		/// Callback functions.
		static void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
		}

		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Window* win = (Window*)glfwGetWindowUserPointer(window);

			Event e;

			switch (action) {
			case GLFW_PRESS: {
				e.KeyPress.keycode = key;
				e.type = EventType::KeyPressEvent;
				win->pushEvent(e);
				break;
			}
			case GLFW_RELEASE: {
				e.KeyRelease.keycode = key;
				e.type = EventType::KeyReleaseEvent;
				win->pushEvent(e);
				break;
			}
			case GLFW_REPEAT: {
				break;
			}
			}
		}

		static void windowResizedCallback(GLFWwindow* window, int width, int height)
		{
			Window* win = (Window*)glfwGetWindowUserPointer(window);
		}
	}
}
