#ifndef BRUSHED_INCLUDE_BRUSHED_WORLD_HPP
#define BRUSHED_INCLUDE_BRUSHED_WORLD_HPP
#include <vector>
#include "brushed/brush.hpp"
#include "brushed/triangle.hpp"
namespace brushed {
	struct world {
		std::vector<brush> brushes;

	public:
		brush& add_brush(const brush& b) {
			return brushes.emplace_back(b);
		}

		void clip_brushes(std::vector<brush*>& brushes) {
			for(size_t i = 0; i < brushes.size(); ++i) {
				if(brushes[i]->type == brush_type::subtractive) {
					for(size_t j = 0; j < i; ++j) {
						if(brushes[j]->type == brush_type::additive) {
							brushes[j]->boundary_planes.erase(
								std::remove_if(brushes[j]->boundary_planes.begin(), brushes[j]->boundary_planes.end(), [&](const plane& p) {
									glm::vec3 point = glm::vec3(p.p) * p.p.w; // Approximate boundary point
									return brushes[i]->is_point_inside(point);
								}),
								brushes[j]->boundary_planes.end());
						}
					}
				}
			}
		}

		std::vector<triangle> generate_mesh_from_brushes(const std::vector<brush>& brushes) {
			std::vector<triangle> triangles;
			for(size_t i = 0; i < brushes.size(); ++i) {
				if(brushes[i].type == brush_type::additive) {
					std::vector<triangle> brush_triangles; // Store the brush's triangles
					for(const auto& plane: brushes[i].boundary_planes) {
						glm::vec3 center = glm::vec3(plane.p) * plane.p.w;
						// Generate a dummy quad for each plane (should be clipped properly)
						glm::vec3 v0 = center + glm::vec3(-1, -1, 0);
						glm::vec3 v1 = center + glm::vec3(1, -1, 0);
						glm::vec3 v2 = center + glm::vec3(1, 1, 0);
						glm::vec3 v3 = center + glm::vec3(-1, 1, 0);
						brush_triangles.push_back({ v0, v1, v2 });
						brush_triangles.push_back({ v2, v3, v0 });
					}
					// Apply subtractive brushes to clip triangles
					for(size_t j = 0; j < i; ++j) {
						if(brushes[j].type == brush_type::subtractive) {
							brush_triangles.erase(
								std::remove_if(brush_triangles.begin(), brush_triangles.end(), [&](const triangle& tri) {
									glm::vec3 center = (tri.v0 + tri.v1 + tri.v2) / 3.0f;
									return brushes[i].is_point_inside(center);
								}),
								brush_triangles.end());
						}
					}
					// Add remaining triangles to the final mesh
					triangles.insert(triangles.end(), brush_triangles.begin(), brush_triangles.end());
				}
			}
			return triangles;
		}
	};
}
#endif