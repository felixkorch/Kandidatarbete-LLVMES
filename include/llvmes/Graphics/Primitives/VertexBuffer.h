#pragma once
#include "llvmes/Graphics/Primitives/VertexBufferLayout.h"
#include <string>
#include <memory>

namespace llvmes {
	namespace graphics {

		class VertexBuffer {
		public:
			VertexBuffer();
			~VertexBuffer();

			void bind();
			void initStaticBufferUsage(const void* data, std::size_t size);
			void initDynamicBufferUsage(std::size_t size);
			void* getInternalPointer() const;
			void releasePointer() const;
			void bindLayout(const VertexBufferLayout& layout);
			void unbind();

		private:
			VertexBuffer(VertexBuffer& other);
		private:

			std::size_t bufferSize;
			unsigned int id;

		};

	}
}