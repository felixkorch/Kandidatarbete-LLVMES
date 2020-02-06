#include "llvmes/Graphics/Window.h"
#include "llvmes/Graphics/Input.h"
#include "llvmes/Graphics/Keycodes.h"
#include "llvmes/Graphics/Rectangle.h"
#include <iostream>

using namespace llvmes::graphics;

int main()
{
	Window window(800, 600, "WindowTest");

	Shader::Err err;

	Shader shader;
	shader.loadFromString(textureShader, err);
	if(err.getType() != Shader::Err::Type::NoError)
		err.print();

	Shader shader2;
	shader2.loadFromString(greyScale, err);
	if(err.getType() != Shader::Err::Type::NoError)
		err.print();

	Rectangle frame(800, 600, 200, 200, 0, 0);

	Texture2D texture(200, 200);
	texture.setColor(70, 90, 150, 200);

	Texture2D texture2(200, 200);
	texture2.setColor(150, 170, 30, 25);

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

		frame.setPosition(x, y);
		frame.draw(shader, texture);
		frame.setPosition(200, 200);
		frame.draw(shader, texture2);

		window.update();
	}
}