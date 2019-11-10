#pragma once
#include <glm/glm.hpp>

class BlackHole
{
public:
	double mass;
	double r_s_km;
	glm::dvec3 position;

	inline static const double G = 6.674 * pow(10, -11);
	const double light_speed = 299792458;

	BlackHole(const double mass, const glm::dvec3& position)
		: mass(mass), position(position)
	{
		r_s_km = (2 * G * mass) / (light_speed * light_speed) / 1000;
	}
};
