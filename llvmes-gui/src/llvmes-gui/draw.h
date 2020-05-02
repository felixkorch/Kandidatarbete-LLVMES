#include <glm/glm.hpp>

#include "llvmes-gui/buffers.h"
#include "llvmes-gui/shader.h"
#include "llvmes-gui/texture.h"

namespace llvmes {
namespace gui {

struct VertexData {
    glm::vec3 vertex;
    glm::vec4 color;
    glm::vec2 uv;
    float tid;
};

class Draw {
   public:
    static void Init();
    static void DeInit();
    static void Begin();
    static void End();
    static void Rectangle(float x, float y, float w, float h);
    static void Rectangle(const glm::vec2& pos, const glm::vec2& size);
    static void UseColor(float r, float g, float b, float a);
    static void UseColor(const glm::vec4& color);
    static void UseTexture(Texture* texture);

   private:
    static VertexArray* vao;
    static IndexBuffer* ibo;
    static Shader* shader;
    static VertexData* data;
    static unsigned int index_count;
    static std::unordered_map<Texture*, unsigned int> textures;
    static Texture* current_texture;
    static glm::vec4 current_color;
};

}  // namespace gui
}  // namespace llvmes