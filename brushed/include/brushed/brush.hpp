#ifndef BRUSHED_INCLUDE_BRUSHED_BRUSH_HPP
#define BRUSHED_INCLUDE_BRUSHED_BRUSH_HPP
#include <vector>
#include "brushed/aabb.hpp"
#include "brushed/plane.hpp"
#include "brushed/transform.hpp"
#include "glm/fwd.hpp"
#include <glm/gtc/matrix_transform.hpp>
namespace brushed {

	struct vec3;
	enum class brush_type {
		additive,
		subtractive,
	};
	struct brush {
		transform world_transform;
		brush_type type;
		std::vector<plane> boundary_planes;
		aabb bounds;
		bool is_point_inside(const glm::vec3& point) const {
			for(const auto& plane: boundary_planes) {
				if(glm::dot(glm::vec3(plane.p), point) > plane.p.w) {
					return false;
				}
			}
			return true;
		}
		bool intersect_ray(glm::vec3 origin, glm::vec3 direction, float& t_closest) const {
			origin = (world_transform.to_mat() * glm::vec4(origin, 1)).xyz();
			direction = (world_transform.to_mat() * glm::vec4(direction, 0)).xyz();
			float t_min = std::numeric_limits<float>::lowest();
			float t_max = std::numeric_limits<float>::max();
			glm::vec3 hit_point;

			for(const auto& plane: boundary_planes) {
				float t;
				if(plane.intersect_ray(origin, direction, t)) {
					hit_point = origin + direction * t;

					// Check if hitPoint satisfies ALL brush boundary planes
					bool inside = true;
					for(const auto& boundary_plane: boundary_planes) {
						if(glm::dot(boundary_plane.normal(), hit_point) > boundary_plane.d()) {
							inside = false;
							break;
						}
					}

					if(inside && t < t_max) {
						t_closest = t;
						return true;
					}
				}
			}

			return false;
		}
	};

	inline aabb compute_bounding_box(const brush& brush) {
		// Initialize min/max values
		auto min_corner = glm::vec3(std::numeric_limits<float>::max());
		auto max_corner = glm::vec3(std::numeric_limits<float>::lowest());

		// Iterate through triplets of planes to compute intersections
		for(size_t i = 0; i < brush.boundary_planes.size(); ++i) {
			for(size_t j = i + 1; j < brush.boundary_planes.size(); ++j) {
				for(size_t k = j + 1; k < brush.boundary_planes.size(); ++k) {
					plane p1 = brush.boundary_planes[i];
					plane p2 = brush.boundary_planes[j];
					plane p3 = brush.boundary_planes[k];
					// Solve system using determinant method
					glm::mat3 a(p1.xyz(), p2.xyz(), p3.xyz());
					float det = determinant(a);
					if(det == 0) {
						continue; // Planes are parallel, no unique solution
					}
					// Compute intersection point
					glm::vec3 rhs(-p1.d(), -p2.d(), -p3.d());
					glm::vec3 intersection = inverse(a) * rhs;
					// Update min/max bounds
					min_corner = glm::min(min_corner, intersection);
					max_corner = glm::max(max_corner, intersection);
				}
			}
		}
		return aabb{ .min = min_corner, .max = max_corner };
	}

	inline brush box(glm::vec3 extents = glm::vec3(256, 256, 256)) {
		return brush{
			.world_transform = {
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 1, 1, 1 } },
			.boundary_planes = { plane{ { 1, 0, 0, -extents.x } }, plane{ { -1, 0, 0, -extents.x } }, plane{ { 0, 1, 0, -extents.y } }, plane{ { 0, -1, 0, -extents.y } }, plane{ { 0, 0, 1, -extents.z } }, plane{ { 0, 0, -1, -extents.z } } }
		};
	}
}
#endif