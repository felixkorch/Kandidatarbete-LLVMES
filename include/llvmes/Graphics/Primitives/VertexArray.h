#pragma once
#include "llvmes/Graphics/Primitives/VertexBuffer.h"
#include <memory>

namespace llvmes{
	namespace graphics {

		class VertexArray {
		public:
			VertexArray();
			~VertexArray();

			void addBuffer(VertexBuffer& vb, const VertexBufferLayout& layout);
			void bind() const;
			void unbind() const;

		private:
			VertexArray(VertexArray& other);
		private:
			unsigned int id;

		};

	}
}