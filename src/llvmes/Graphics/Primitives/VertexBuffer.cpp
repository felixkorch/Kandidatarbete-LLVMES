#include "llvmes/Graphics/Primitives/VertexBuffer.h"

namespace llvmes {
	namespace graphics {
		VertexBuffer::VertexBuffer()
			: bufferSize(0)
		{
			glGenBuffers(1, &id);
		}

		VertexBuffer::~VertexBuffer()
		{
			glDeleteBuffers(1, &id);
		}


		void VertexBuffer::bind()
		{
			glBindBuffer(GL_ARRAY_BUFFER, id);
		}

		void VertexBuffer::initStaticBufferUsage(const void* data, std::size_t size)
		{
			bufferSize = size;
			glBindBuffer(GL_ARRAY_BUFFER, id);
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		}

		void VertexBuffer::initDynamicBufferUsage(std::size_t size)
		{
			bufferSize = size;
			glBindBuffer(GL_ARRAY_BUFFER, id);
			glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		}

		void* VertexBuffer::getInternalPointer() const
		{
			//return glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
			return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		void VertexBuffer::releasePointer() const
		{
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}

		void VertexBuffer::bindLayout(const VertexBufferLayout& layout)
		{
			const auto& elements = layout.getElements();
			std::size_t offset = 0;
			for (unsigned int i = 0; i < elements.size(); i++) {
				const auto& element = elements[i];
				glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset);
				glEnableVertexAttribArray(i);
				offset += element.count * VertexBufferElement::getSizeOfType(element.type);
			}
		}

		void VertexBuffer::unbind()
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		std::shared_ptr<VertexBuffer> VertexBuffer::create()
		{
			return std::shared_ptr<VertexBuffer>(new VertexBuffer);
		}

	}
}