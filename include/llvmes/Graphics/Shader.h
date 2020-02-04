#pragma once
#include <string>
#include <iostream>
#include <memory>

namespace llvmes {
	namespace graphics {

		class Shader {
		public:

			struct ProgramSource {
				std::string VertexSource;
				std::string FragmentSource;
			};

			class Err {
			public:
				enum class Type {
					FileNotFound,
					EmptyString,
					InvalidString,
					NoError
				};

				Err()
					: msg()
					, type(Type::NoError)
				{}

				Type getType() { return type; }
				void setType(Type type) { this->type = type; };
				void setErrorMessage(const std::string& msg) { this->msg = msg; }
				void print() { std::cout << msg << std::endl; }

			private:
				std::string msg;
				Type type;
			};

			static std::shared_ptr<Shader> createFromFile(const std::string& path, Shader::Err& err);
			static std::shared_ptr<Shader> createFromString(const std::string& string, Shader::Err& err);

			void bind() const;
			void unbind() const;


		private:

			/// Makes the class non-constructible
			Shader() = default;

			/// Makes the class non-copyable
			Shader(Shader& other);

			/// Parses a string and creates two strings, the vertex and fragment part.
			Shader::ProgramSource parseShader(std::stringstream& str);

			/// Used by "createShader" to compile the programs
			unsigned int compileShader(unsigned int type, const std::string& source);

			/// Links the fragment and vertex parts into a shader program
			void createShader(const std::string& vertexShader, const std::string& fragmentShader);

		private:
			unsigned int id;
		};

	}
}