#pragma once
#include "glad/glad.h"
#include <vector>

namespace llvmes {
	namespace graphics {
		struct VertexBufferElement {
			unsigned int type;
			unsigned int count;
			unsigned int normalized;

			static unsigned int getSizeOfType(unsigned int type)
			{
				switch (type) {
				case GL_FLOAT:         return 4;
				case GL_UNSIGNED_INT:  return 4;
				case GL_UNSIGNED_BYTE: return 1;
				default:break;
				}
				return 0;
			}
		};

		class VertexBufferLayout {
		public:
			VertexBufferLayout()
				:stride(0) {}

			template<typename T>
			void push(unsigned int count);

			const std::vector<VertexBufferElement> getElements() const { return elements; }

			unsigned int getStride() const { return stride; }

		private:
			std::vector<VertexBufferElement> elements;
			unsigned int stride;
		};

		template<>
		inline void VertexBufferLayout::push<float>(unsigned int count)
		{
			elements.push_back({ GL_FLOAT, count, GL_FALSE });
			stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
		}

		template<>
		inline void VertexBufferLayout::push<unsigned int>(unsigned int count)
		{
			elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
			stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
		}

		template<>
		inline void VertexBufferLayout::push<unsigned char>(unsigned int count)
		{
			elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
			stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
		}
	}
}