#include "llvmes/Graphics/Primitives/IndexBuffer.h"
#include "glad/glad.h"

namespace llvmes {
	namespace graphics {

		IndexBuffer::IndexBuffer()
			: count(0)
			, id(0)
		{}

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

		std::shared_ptr<IndexBuffer> IndexBuffer::create(const unsigned int* data, unsigned int count)
		{
			std::shared_ptr<IndexBuffer> ibo = std::shared_ptr<IndexBuffer>(new IndexBuffer);
			ibo->count = count;
			glGenBuffers(1, &ibo->id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
			return ibo;
		}

	}
}