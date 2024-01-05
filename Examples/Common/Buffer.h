#pragma once

namespace UniformBuf
{

enum class UniformType
{
    ModelViewProjection = 0,
    Lighting
};

} // namespace Uniform

namespace ComputeBuf
{

enum class BufferType
{
    Input = 0,
    Output,
    Map
};
    
} // namespace Compute