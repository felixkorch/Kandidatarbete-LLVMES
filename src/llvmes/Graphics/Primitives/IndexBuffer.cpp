#include "llvmes/Graphics/Primitives/IndexBuffer.h"
#include "glad/glad.h"

namespace llvmes {
	namespace graphics {

		IndexBuffer::IndexBuffer()
			: count(0)
			, id(0)
		{
			glGenBuffers(1, &id);
		}

		IndexBuffer::~IndexBuffer()
		{
			glDeleteBuffers(1, &id);
		}

		void IndexBuffer::bind() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		}

		void IndexBuffer::unbind() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		void IndexBuffer::load(const unsigned int* data, unsigned int count)
		{
			this->count = count;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
		}

	}
}