#include <vector>

namespace llvmes {
namespace gui {

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_2_BYTES 0x1407
#define GL_3_BYTES 0x1408
#define GL_4_BYTES 0x1409
#define GL_DOUBLE 0x140A
#define GL_TRUE 1
#define GL_FALSE 0

struct VertexBufferElement {
    unsigned int type;
    unsigned int count;
    unsigned int normalized;

    static unsigned int GetSizeOfType(unsigned int type)
    {
        switch (type) {
            case GL_FLOAT:
                return 4;
            case GL_UNSIGNED_INT:
                return 4;
            case GL_UNSIGNED_BYTE:
                return 1;
            default:
                break;
        }
        return 0;
    }
};

class VertexBufferLayout {
   private:
    std::vector<VertexBufferElement> elements;
    unsigned int stride;

   public:
    VertexBufferLayout() : stride(0) {}

    template <typename T>
    void Push(unsigned int count);

    const std::vector<VertexBufferElement> GetElements() const { return elements; }

    unsigned int GetStride() const { return stride; }
};

template <>
inline void VertexBufferLayout::Push<float>(unsigned int count)
{
    elements.push_back({GL_FLOAT, count, GL_FALSE});
    stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template <>
inline void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
    elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}

template <>
inline void VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
    elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
    stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
}

class IndexBuffer {
   public:
    IndexBuffer(const unsigned int* data, unsigned int count);
    ~IndexBuffer();

    // Non-copyable
    IndexBuffer(const IndexBuffer& other) = delete;
    IndexBuffer& operator=(const IndexBuffer& other) = delete;

    void Bind();
    void Unbind();

   private:
    unsigned int unique_id;
};

enum class BufferUsage { DYNAMIC, STATIC, NONE };

class VertexBuffer {
   public:
    VertexBuffer();
    ~VertexBuffer();
    // Non-copyable
    VertexBuffer(const VertexBuffer& other) = delete;
    VertexBuffer& operator=(const VertexBuffer& other) = delete;

    void SetType(BufferUsage usage);
    void Load(size_t size, const void* data = nullptr);
    void* GetInternalPointer();
    void ReleasePointer();
    void Bind();
    void Unbind();

   private:
    unsigned int unique_id;
    BufferUsage type;
    size_t buffer_size;
};

class VertexArray {
   public:
    VertexArray();
    ~VertexArray();

    // Make non-copyable
    VertexArray(VertexArray&& other) = delete;
    VertexArray& operator=(VertexArray&& other) = delete;

    void AddBuffer(VertexBuffer* vb, const VertexBufferLayout& layout);
    void Bind();
    void Unbind();

    VertexBuffer& GetBuffer(int index = 0) { return *buffers[index]; }

   private:
    unsigned int unique_id;
    std::vector<VertexBuffer*> buffers;
};

}  // namespace gui
}  // namespace llvmes