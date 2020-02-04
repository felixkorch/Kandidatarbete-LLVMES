#include "llvmes/Graphics/Window.h"
#include "llvmes/Graphics/Input.h"
#include "llvmes/Graphics/Keycodes.h"
#include "llvmes/Graphics/Texture.h"
#include <iostream>

using namespace llvmes::graphics;

int main()
{
	Window window(800, 600, "WindowTest");

	Shader::Err err;
	auto shader = Shader::createFromString(textureShader, err);
	if (!shader)
		err.print();

	auto shader2 = Shader::createFromString(greyScale, err);
	if (!shader2)
		err.print();

	Texture frame(800, 600, 200, 200, 0, 0);

	float x = 0;
	float y = 0;

	while (window.isOpen()) {
		window.clear();

		Event e;
		while (window.pollEvent(e)) {
			if (e.type == EventType::KeyPressEvent) {
				std::cout << "Key pressed: " << e.KeyPress.keycode << std::endl;
			}
		}

		if(Input::isKeyPressed(window.getNativeWindow(), GLFW_KEY_W)) {
			y += 1;
		}
		if (Input::isKeyPressed(window.getNativeWindow(), GLFW_KEY_A)) {
			x -= 1;
		}
		if (Input::isKeyPressed(window.getNativeWindow(), GLFW_KEY_S)) {
			y -= 1;
		}
		if (Input::isKeyPressed(window.getNativeWindow(), GLFW_KEY_D)) {
			x += 1;
		}

		frame.setColor(150, 70, 90, 77);
		frame.setPosition(x, y);
		frame.draw(shader);
		frame.setColor(165, 20, 200, 90);
		frame.setPosition(200, 200);
		frame.draw(shader);

		window.update();
	}
}