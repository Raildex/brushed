#ifndef BRUSHED_INCLUDE_BRUSHED_PLANE_HPP
#define BRUSHED_INCLUDE_BRUSHED_PLANE_HPP
#include <glm/glm.hpp>
namespace brushed {

	struct plane {
		glm::vec4 p;
		plane inverted() {
			return plane{
				.p = p * -1.0f
			};
		}
		glm::vec3 xyz() const {
			return p.xyz();
		}

		glm::vec3 normal() const {
			return xyz();
		}

		float d() const {
			return p.w;
		}

		bool intersect_ray(const glm::vec3& origin, const glm::vec3& direction, float& t) const {
			float denom = glm::dot(normal(), direction);
			if(glm::abs(denom) < 1e-6f)
				return false; // Parallel to the plane

			t = (d() - glm::dot(normal(), origin)) / denom;
			return t > 0;
		}
	};

	inline plane create_plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
		glm::vec3 v0 = p1 - p0;
		glm::vec3 v1 = p2 - p0;
		glm::vec3 n = glm::cross(v0, v1);
		float d = glm::dot(n, p0);
		return plane{
			.p = glm::vec4(n, d)
		};
	}
}
#endif