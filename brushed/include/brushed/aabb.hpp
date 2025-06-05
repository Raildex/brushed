#ifndef BRUSHED_INCLUDE_BRUSHED_AABB_HPP
#define BRUSHED_INCLUDE_BRUSHED_AABB_HPP
#include <glm/glm.hpp>
#include <algorithm>
namespace brushed {
	struct aabb {
		glm::vec3 min;
		glm::vec3 max;

		bool intersect_ray(const glm::vec3& origin, const glm::vec3& direction, float& tMin, float& tMax) const {
			glm::vec3 inv_dir = 1.0f / direction;
			glm::vec3 t1 = (min - origin) * inv_dir;
			glm::vec3 t2 = (max - origin) * inv_dir;

			glm::vec3 t_min = glm::min(t1, t2);
			glm::vec3 t_max = glm::max(t1, t2);

			tMin = std::max({ t_min.x, t_min.y, t_min.z });
			tMax = std::min({ t_max.x, t_max.y, t_max.z });

			return tMax >= tMin && tMax > 0;
		};
	};
	inline aabb merge(const aabb& a, const aabb& b) {
		return {
			.min = glm::min(a.min, b.min),
			.max = glm::max(a.max, b.max)
		};
	}
}
#endif