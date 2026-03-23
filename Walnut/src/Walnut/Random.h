#pragma once

#include <random>
#include "ExportConfig.h"

namespace Walnut {

	class Walnut_API Random
	{
	public:
		static void Init();
		static uint32_t UInt();
		static uint32_t UInt(uint32_t min, uint32_t max);
		static float Float();
		static std::array<float, 3> Vec3();
		static std::array<float, 3> Vec3(float min, float max);

	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

}


