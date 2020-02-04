#include "llvmes/Graphics/Primitives/Texture2D.h"
#include "glad/glad.h"
#include <vector>

namespace llvmes {
	namespace graphics {
		
		Texture2D::Texture2D(int width, int height)
			: width(width)
			, height(height)
		{
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		Texture2D::~Texture2D()
		{
			glDeleteTextures(1, &id);
		}

		void Texture2D::bind(unsigned slot) const
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, id);
		}

		void Texture2D::unbind() const
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void Texture2D::setData(void* pixels)
		{
			glBindTexture(GL_TEXTURE_2D, id);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA,
				GL_UNSIGNED_BYTE, pixels);
		}

		void Texture2D::setColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
		{
			std::vector<char> data(width * height * 4);
			int i, j;
			for (i = 0; i < width; i++) {
				for (j = 0; j < height; j++) {
					data[i * height * 4 + j * 4 + 0] = r;
					data[i * height * 4 + j * 4 + 1] = g;
					data[i * height * 4 + j * 4 + 2] = b;
					data[i * height * 4 + j * 4 + 3] = a;
				}
			}
			setData(data.data());
		}

		std::shared_ptr<Texture2D> Texture2D::create(int width, int height)
		{
			return std::shared_ptr<Texture2D>(new Texture2D(width, height));
		}

	}
}