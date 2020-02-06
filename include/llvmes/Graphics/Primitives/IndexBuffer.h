#pragma once
#include <memory>

namespace llvmes {
	namespace graphics {

		class IndexBuffer {
		public:
			IndexBuffer();
			~IndexBuffer();

			void bind() const;
			void unbind() const;
			void load(const unsigned int* data, unsigned int count);

		private:
			IndexBuffer(IndexBuffer& other);

			unsigned int count;
			unsigned int id;
		};

	}
}