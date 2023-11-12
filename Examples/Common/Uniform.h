#pragma once

#include <array>

/**
 * The same structure as in the shader, replicated in C++
 */
struct MyUniforms {
	// offset = 0 * sizeof(vec4f) -> OK
	std::array<float, 4> color;

	// offset = 16 = 4 * sizeof(f32) -> OK
	float time;

	// Add padding to make sure the struct is host-shareable
	float _pad[3];
};
// Have the compiler check byte alignment
static_assert(sizeof(MyUniforms) % 16 == 0);