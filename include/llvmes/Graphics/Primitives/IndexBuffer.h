#pragma once
#include <memory>

namespace llvmes {
	namespace graphics {

		class IndexBuffer {
		public:
			~IndexBuffer();

			void bind() const;
			void unbind() const;

			static std::shared_ptr<IndexBuffer> create(const unsigned int* data, unsigned int count);

		private:
			IndexBuffer();
			IndexBuffer(IndexBuffer& other) = delete;

			unsigned int count;
			unsigned int id;
		};

	}
}