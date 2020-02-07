#include "llvmes/Graphics/Shader.h"
#include "glad/glad.h"
#include <iostream>
#include <sstream>
#include <fstream>

namespace llvmes {
	namespace graphics {

		unsigned int Shader::compileShader(unsigned int type, const std::string& source, Shader::Err& err)
		{
			unsigned int id = glCreateShader(type);
			const char* src = source.c_str();
			glShaderSource(id, 1, &src, nullptr);
			glCompileShader(id);

			int result;
			glGetShaderiv(id, GL_COMPILE_STATUS, &result);

			if (result == GL_FALSE) {
				int length;
				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
				char* message = (char*)alloca(length * sizeof(char));
				glGetShaderInfoLog(id, length, &length, message);
				std::cout << message << ", " << type << std::endl;
				glDeleteShader(id);
				return 0;
			}

			return id;
		}

		void Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader, Shader::Err& err)
		{
			if (vertexShader.empty() || fragmentShader.empty()) {
				err.setType(Err::Type::EmptyString);
				err.setErrorMessage("Vertex or Fragment shader empty.");
			}

			unsigned int program = glCreateProgram();

			if (program == 0) {
				err.setType(Err::Type::ProgramNotCreated);
				err.setErrorMessage("Create Program failed.");
			}

			unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader, err);
			unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader, err);

			glAttachShader(program, vs);
			glAttachShader(program, fs);
			glLinkProgram(program);
			glValidateProgram(program);

			glDeleteShader(vs);
			glDeleteShader(fs);

			id = program;
		}

		Shader::ProgramSource Shader::parseShader(std::stringstream& str, Shader::Err& err)
		{
			enum class ShaderType {
				NONE = -1, VERTEX = 0, FRAGMENT = 1
			};

			std::string line;
			std::stringstream ss[2];
			ShaderType type = ShaderType::NONE;

			while (std::getline(str, line)) {
				if (line == "")
					continue;
				if (line.find("#shader") != std::string::npos) {

					if (line.find("vertex") != std::string::npos)
						type = ShaderType::VERTEX;

					else if (line.find("fragment") != std::string::npos)
						type = ShaderType::FRAGMENT;
				}
				else {
					if (type == ShaderType::NONE) {
						err.setType(Err::Type::InvalidShaderFormat);
						err.setErrorMessage("Invalid shader format. Needs to contain vertex / fragments directives.");
						return {};
					}
					ss[(int)type] << line << '\n';
				}
			}

			return { ss[0].str(), ss[1].str() };
		}

		void Shader::loadFromFile(const std::string& path, Shader::Err& err)
		{
			std::ifstream stream(path);
			if (!stream.good()) {
				err.setType(Err::Type::FileNotFound);
				err.setErrorMessage("Shader not found given the path:" + path);
			}

			std::stringstream str;
			str << stream.rdbuf();

			Shader::ProgramSource source = parseShader(str, err);
			createShader(source.VertexSource, source.FragmentSource, err);
		}

		void Shader::loadFromString(const std::string& string, Shader::Err& err)
		{
			std::stringstream str(string);
			Shader::ProgramSource source = parseShader(str, err);
			createShader(source.VertexSource, source.FragmentSource, err);
		}

		void Shader::bind() const
		{
			glUseProgram(id);
		}

		void Shader::unbind() const
		{
			glUseProgram(0);
		}

	}
}