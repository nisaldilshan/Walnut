#include "Random.h"

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
    glm::vec3 Random::Vec3()
    {
		return glm::vec3(Float(), Float(), Float());
	}
    glm::vec3 Random::Vec3(float min, float max)
    {
		return glm::vec3(Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min);
	}
    glm::vec3 Random::InUnitSphere()
    {
		return glm::normalize(Vec3(-1.0f, 1.0f));
	}
}