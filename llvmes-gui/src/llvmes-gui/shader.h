#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

namespace llvmes {
namespace gui {

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

class Shader {
   public:
    Shader(const char* program);
    ~Shader();

    void Bind();
    void Unbind();

    void SetUniform1i(const std::string& name, int v0);
    void SetUniform1iv(const std::string& name, int count, const int* value);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(const std::string& name, const glm::mat4& mat);
    void SetUniform1f(const std::string& name, float v0);
    void SetUniform3f(const std::string& name, const glm::vec3& vec3);
    int GetUniformLocation(const std::string& name);

   private:
    unsigned int unique_id;
    std::string file_path;
    std::unordered_map<std::string, int> uniform_cache;
};

}  // namespace gui
}  // namespace llvmes