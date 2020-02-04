#pragma once
#include "llvmes/Graphics/Primitives/VertexBuffer.h"
#include <memory>

namespace llvmes{
	namespace graphics {

		class VertexArray {
		public:
			~VertexArray();

			void addBuffer(const std::shared_ptr<VertexBuffer>& vb, const VertexBufferLayout& layout);
			void bind() const;
			void unbind() const;

			static std::shared_ptr<VertexArray> create();

		private:
			VertexArray();
			VertexArray(VertexArray& other);
		private:
			unsigned int id;

		};

	}
}