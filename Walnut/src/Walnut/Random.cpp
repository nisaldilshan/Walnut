#include "Random.h"
#include <array>

namespace Walnut {

	std::mt19937 Random::s_RandomEngine;
	std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution;

    void Random::Init()
    {
		s_RandomEngine.seed(std::random_device()());
    }

	uint32_t Random::UInt()
    {
        return s_Distribution(s_RandomEngine);
    }

    uint32_t Random::UInt(uint32_t min, uint32_t max)
    {
        return min + (s_Distribution(s_RandomEngine) % (max - min + 1));
    }

    float Random::Float()
    {
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

    std::array<float, 3> Random::Vec3()
    {
		return { Float(), Float(), Float() };
	}

    std::array<float, 3> Random::Vec3(float min, float max)
    {
		return { Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min };
	}
}