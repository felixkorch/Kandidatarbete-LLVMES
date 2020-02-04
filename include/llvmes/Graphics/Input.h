#pragma once
#include "llvmes/Graphics/Window.h"

namespace llvmes {
	namespace graphics {
		class Input {
		public:
			static bool isKeyPressed(void* window, int key);
		};
	}
}