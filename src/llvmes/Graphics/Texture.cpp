#include "llvmes/Graphics/Texture.h"

namespace llvmes {
	namespace graphics {

		Texture::Texture(int windowWidth, int windowHeight, float width, float height, float x, float y)
			: windowWidth(windowWidth)
			, windowHeight(windowHeight)
			, width(width)
			, height(height)
			, x(x)
			, y(y)
		{
			unsigned int indices[] = {
				0, 1, 3,
				1, 2, 3
			};

			texture = Texture2D::create(width, height);
			vao = VertexArray::create();
			vao->bind();

			vbo = VertexBuffer::create();
			vbo->initDynamicBufferUsage(sizeof(VertexData) * 4);
			ibo = IndexBuffer::create(indices, 6);

			VertexBufferLayout layout;
			layout.push<float>(2); // Position
			layout.push<float>(2); // Texture coordinates
			vao->addBuffer(vbo, layout);

			update();
		}

		void Texture::setData(char* pixels)
		{
			texture->setData(pixels);
		}

		void Texture::setSize(float width, float height)
		{
			this->width = width;
			this->height = height;
			update();
		}

		void Texture::setPosition(float x, float y)
		{
			this->x = x;
			this->y = y;
			update();
		}

		void Texture::setColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
		{
			texture->setColor(r, g, b, a);
		}

		void Texture::draw(const std::shared_ptr<Shader>& shader)
		{
			shader->bind();
			texture->bind(0);
			vao->bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		}

		void Texture::update()
		{
			vbo->bind();
			VertexData* vertices = (VertexData*)vbo->getInternalPointer();
			vertices->pos = normalize({ x + width, y + height }); // top right
			vertices->uv = { 1.0f, 1.0f };
			vertices++;
			vertices->pos = normalize({ x + width, y }); // bottom right
			vertices->uv = { 1.0f, 0.0f };
			vertices++;
			vertices->pos = normalize({ x, y }); // bottom left
			vertices->uv = { 0.0f, 0.0f };
			vertices++;
			vertices->pos = normalize({ x, y + height }); // top left 
			vertices->uv = { 0.0f, 1.0f };
			vbo->releasePointer();
		}

	}
}