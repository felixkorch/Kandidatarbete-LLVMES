#pragma once
#include <memory>
namespace llvmes {
	namespace graphics {

		class Texture2D {
		public:
			Texture2D(int width, int height);
			~Texture2D();

			void bind(unsigned slot) const;
			void unbind() const;
			void setData(void* pixels);
			void setColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
			unsigned int getID() { return id; }
			int getWidth() { return width; }
			int getHeight() { return height; }

		private:
			Texture2D(Texture2D& other);
		private:
			unsigned int id;
			int width, height;

		};

	}
}