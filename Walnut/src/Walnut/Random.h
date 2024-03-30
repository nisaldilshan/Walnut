#pragma once

#include <random>

#include "ExportConfig.h"

#include <glm/ext.hpp>

namespace Walnut {

	class Walnut_API Random
	{
	public:
		static void Init();

		static uint32_t UInt();

		static uint32_t UInt(uint32_t min, uint32_t max);


		static float Float();
		

		static glm::vec3 Vec3();
		

		static glm::vec3 Vec3(float min, float max);
		

		static glm::vec3 InUnitSphere();
		
	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

}


