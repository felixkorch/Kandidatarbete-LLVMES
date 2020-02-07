#pragma once
#include "llvmes/Graphics/Event.h"
#include <string>
#include <queue>

namespace llvmes {
	namespace graphics {

		class Window {
		public:
			Window(int width, int height, const std::string& title, bool resizable = false);
			bool isOpen() const;
			void clear();
			void update();
			bool pollEvent(Event& e);
			void pushEvent(Event& e);
			void* getNativeWindow() { return glfwWindow; }

		private:
			std::string title;
			void* glfwWindow;
			int width, height;
			bool resizable;
			std::queue<Event> eventQueue;
		};

	}
}