#pragma once

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../../vendor/stb_image/stb_image.h"

namespace Texture
{

class TextureHandle
{
public:
    TextureHandle(unsigned char* textureData, int width, int height, uint32_t mipMapLevelCount)
        : m_textureData(textureData)
        , m_texWidth(width)
        , m_texHeight(height)
        , m_mipMapLevelCount(mipMapLevelCount)
    {}

    ~TextureHandle()
    {
        stbi_image_free(m_textureData);
    }

    unsigned char* GetData() const
    {
        return m_textureData;
    }

    int GetWidth() const
    {
        return m_texWidth;
    }

    int GetHeight() const 
    {
        return m_texHeight;
    }

    uint32_t GetMipLevelCount() const 
    {
        return m_mipMapLevelCount;
    }
private:
    unsigned char* m_textureData = nullptr;
    int m_texWidth = 0;
	int m_texHeight = 0; 
	uint32_t m_mipMapLevelCount = 0;
};

namespace fs = std::filesystem;

// Equivalent of std::bit_width that is available from C++20 onward
inline uint32_t bit_width(uint32_t m) {
    if (m == 0) return 0;
    else { uint32_t w = 0; while (m >>= 1) ++w; return w; }
}

std::unique_ptr<TextureHandle> loadTexture(const fs::path &path)
{
    int channels;
    int width;
    int height;
    unsigned char *pixelData = stbi_load(path.string().c_str(), &width, &height, &channels, 4 /* force 4 channels */);
    if (nullptr == pixelData)
    {
        assert(false);
        return nullptr;
    }

    uint32_t mipMapLevelCount = bit_width(std::max(width, height));

    return std::make_unique<TextureHandle>(pixelData, width, height, mipMapLevelCount);
}




}