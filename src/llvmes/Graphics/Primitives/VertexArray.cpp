#include "llvmes/Graphics/Primitives/VertexArray.h"
#include "llvmes/Graphics/Primitives/VertexBuffer.h"
#include "glad/glad.h"

namespace llvmes {
	namespace graphics {

		VertexArray::VertexArray()
		{
			glGenVertexArrays(1, &id);
		}

		VertexArray::~VertexArray()
		{
			glDeleteVertexArrays(1, &id);
		}

		void VertexArray::addBuffer(const std::shared_ptr<VertexBuffer>& vb, const VertexBufferLayout& layout)
		{
			bind();
			vb->bind();
			const auto& elements = layout.getElements();
			std::size_t offset = 0;
			for (unsigned int i = 0; i < elements.size(); i++) {
				const auto& element = elements[i];
				glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset);
				glEnableVertexAttribArray(i);
				offset += element.count * VertexBufferElement::getSizeOfType(element.type);
			}

			//unbind();
		}

		void VertexArray::bind() const
		{
			glBindVertexArray(id);
		}

		void VertexArray::unbind() const
		{
			glBindVertexArray(0);
		}

		std::shared_ptr<VertexArray> VertexArray::create()
		{
			return std::shared_ptr<VertexArray>(new VertexArray);
		}

	}
}