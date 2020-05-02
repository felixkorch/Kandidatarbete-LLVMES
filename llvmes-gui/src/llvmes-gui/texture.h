namespace llvmes {
namespace gui {

enum class TextureWrap { CLAMP, CLAMP_TO_BORDER, CLAMP_TO_EDGE, REPEAT, MIRRORED_REPEAT };
enum class TextureFormat { RGBA, RGB, RGBA32F, LUMINANCE, LUMINANCE_ALPHA };
enum class TextureFilter { NEAREST, LINEAR };

struct TextureType {
    TextureWrap wrap;
    TextureFormat format;
    TextureFilter filter;
};

static constexpr TextureType GetDefaultType = {
    TextureWrap::CLAMP_TO_EDGE, TextureFormat::RGBA, TextureFilter::NEAREST};

class Texture {
   public:
    Texture(int width, int height, TextureType type = GetDefaultType);
    ~Texture();

    // Non-copyable
    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    void Bind(unsigned int slot = 0);
    void Unbind();
    void SetData(void* data);
    void Resize(int width, int height);

   private:
    unsigned int unique_id;
    int width, height;
    TextureType type;
};

}  // namespace gui
}  // namespace llvmes