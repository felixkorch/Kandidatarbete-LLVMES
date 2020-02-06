#include "llvmes/Graphics/Primitives/IndexBuffer.h"
#include "llvmes/Graphics/Primitives/VertexBuffer.h"
#include "llvmes/Graphics/Primitives/VertexArray.h"
#include "llvmes/Graphics/Primitives/Texture2D.h"
#include "llvmes/Graphics/Shader.h"
#include <string>

namespace llvmes {
	namespace graphics {

		const std::string textureShader =
			"#shader vertex\n"
			"#version 330 core\n"
			"layout(location = 0) in vec2 position;\n"
			"layout(location = 1) in vec2 uv;\n"
			"out DATA\n"
			"{\n"
				"vec2 position;\n"
				"vec2 uv;\n"
			"} vs_out;\n"
			"void main() {\n"
				"gl_Position = vec4(position, 0.0, 1.0);\n"
				"vs_out.position = position;\n"
				"vs_out.uv = uv;\n"
			"}\n"
			"#shader fragment\n"
			"#version 330 core\n"
			"uniform sampler2D sampler;\n"
			"in DATA\n"
			"{\n"
				"vec2 position;\n"
				"vec2 uv;\n"
			"} fs_in;\n"
			"layout(location = 0) out vec4 color;\n"
			"void main() {\n"
				"color = texture(sampler, fs_in.uv);\n"
			"}\n";

		const std::string greyScale =
			"#shader vertex\n"
			"#version 330 core\n"
			"layout(location = 0) in vec2 position;\n"
			"layout(location = 1) in vec2 uv;\n"
			"out DATA\n"
			"{\n"
				"vec2 position;\n"
				"vec2 uv;\n"
			"} vs_out;\n"
			"void main() {\n"
				"gl_Position = vec4(position, 0.0, 1.0);\n"
				"vs_out.position = position;\n"
				"vs_out.uv = uv;\n"
			"}\n"
			"#shader fragment\n"
			"#version 330 core\n"
			"uniform sampler2D sampler;\n"
			"in DATA\n"
			"{\n"
				"vec2 position;\n"
				"vec2 uv;\n"
			"} fs_in;\n"
			"layout(location = 0) out vec4 color;\n"
			"void main() {\n"
				"vec4 fragColor = texture(sampler, fs_in.uv);"
				"float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;\n"
				"color = vec4(average, average, average, 1.0);\n"
			"}\n";

		class Rectangle {
		public:
			Rectangle(int windowWith, int windowHeight, float width, float height, float x, float y);
			void setData(char* pixels);
			void setSize(float width, float height);
			void setPosition(float x, float y);
			void setColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
			void draw(Shader& shader, Texture2D& texture);
		private:

			struct Vec2 {
				float x, y;
			};

			struct VertexData {
				Vec2 pos;
				Vec2 uv;
			};

			Vec2 normalize(Vec2 xy) { return { ((xy.x * 2) / windowWidth) - 1, ((xy.y * 2) / windowHeight) - 1 }; }
			void update();

			int windowWidth, windowHeight;
			float width, height;
			float x, y;
			VertexArray vao;
			VertexBuffer vbo;
			IndexBuffer ibo;
		};

	}
}