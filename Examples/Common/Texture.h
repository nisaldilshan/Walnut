#pragma once

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../../vendor/stb_image/stb_image.h"

namespace Texture
{
namespace fs = std::filesystem;

// Equivalent of std::bit_width that is available from C++20 onward
inline uint32_t bit_width(uint32_t m) {
    if (m == 0) return 0;
    else { uint32_t w = 0; while (m >>= 1) ++w; return w; }
}

unsigned char* loadTexture(const fs::path &path, int& width, int& height, uint32_t& mipMapLevelCount)
{
    int channels;
    unsigned char *pixelData = stbi_load(path.string().c_str(), &width, &height, &channels, 4 /* force 4 channels */);
    if (nullptr == pixelData)
    {
        assert(false);
        return nullptr;
    }

    mipMapLevelCount = bit_width(std::max(width, height));

    return pixelData;
}

void freeTexture(unsigned char* texture)
{
    stbi_image_free(texture);
}

}