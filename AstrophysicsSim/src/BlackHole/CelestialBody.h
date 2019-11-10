#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <future>

class CelestialBody
{
public:

	CelestialBody(const glm::dvec3& position, const glm::dvec3& velocity, const double mass, const std::string& name)
		: position(position),
		  velocity(velocity),
		  mass(mass),
		  name(name)
	{
		lastPosition = position;
	}

	glm::dvec3 position; //kilometer
	glm::dvec3 lastPosition;
	glm::dvec3 velocity; // meter
	double mass;
	std::string name;
	const double G = 6.674 * pow(10, -11);

	void iterate(double stepLength, std::vector<CelestialBody*> bodies)
	{
		glm::dvec3 force = glm::dvec3(0);

		for (auto& body : bodies)
		{
			if (body->name == name)
				continue;
			glm::dvec3 direction = body->lastPosition - position;
			double r = glm::length(direction);
			// direction = glm::normalize(direction);
			double f = (G * mass * body->mass) / (r * 1000 * r * 1000);
			double cosTheta = glm::dot(direction, glm::dvec3(1, 0, 0)) / r;
			double x = f * cosTheta;
			double z = sqrt(f * f - x * x);

			if(direction.z < 0.f)
			{
				z = -z;
			}

			force += glm::dvec3(x, 0.f, z);
		}

		glm::dvec3 dp = stepLength * force;
		glm::dvec3 dv = dp / mass;
		velocity += dv;
		position += velocity / 1000.0 * stepLength;
	}


	static void batch_iterate(double stepLength, uint32_t steps, std::vector<CelestialBody*>& bodies)
	{
		for (uint32_t i = 0; i < steps; i++)
		{
			for (size_t j = 0; j < bodies.size(); j++)
			{
				bodies[j]->iterate(stepLength, bodies);
			}


			for (size_t j = 0; j < bodies.size(); j++)
			{
				bodies[j]->lastPosition = bodies[j]->position;
			}
		}
	}
};
