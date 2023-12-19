#pragma once

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../../vendor/stb_image/stb_image.h"

namespace Texture
{
namespace fs = std::filesystem;

unsigned char* loadTexture(const fs::path &path, int& width, int& height)
{
    int channels;
    unsigned char *pixelData = stbi_load(path.string().c_str(), &width, &height, &channels, 4 /* force 4 channels */);
    if (nullptr == pixelData)
    {
        assert(false);
        return nullptr;
    }
    
    return pixelData;
}

void freeTexture(unsigned char* texture)
{
    stbi_image_free(texture);
}

}