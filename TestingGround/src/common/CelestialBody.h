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

	glm::dvec3 position;
	glm::dvec3 lastPosition;
	glm::dvec3 velocity;
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
			double f = (G * mass * body->mass) / (r * r);
			double cosPhi = glm::dot(direction, glm::dvec3(1, 0, 0)) / r;
			double cosTheta = glm::dot(direction, glm::dvec3(0, 0, 1)) / r;
			double sinTheta = 1 - (cosTheta * cosTheta);
			double sinPhi = 1 - (cosPhi * cosPhi);
			double x = f * sinTheta * cosPhi;
			double y = f * sinTheta * sinPhi;
			double z = f * cosTheta;

			if (direction.y < 0.f)
				y = -y;

			force += glm::dvec3(x, y, z);
		}

		glm::dvec3 dp = stepLength * force;
		glm::dvec3 dv = dp / mass;
		velocity += dv;
		position += velocity * stepLength;
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
