#ifndef BRUSHED_INCLUDE_BRUSHED_BVH_HPP
#define BRUSHED_INCLUDE_BRUSHED_BVH_HPP
#include "brushed/aabb.hpp"
#include "brushed/brush.hpp"
#include <algorithm>
#include <memory>
#include <vector>
namespace brushed {

	struct bvh_node {
		aabb bounds;
		std::unique_ptr<bvh_node> left = nullptr;
		std::unique_ptr<bvh_node> right = nullptr;
		std::vector<brush*> brushes;
		bvh_node(const std::vector<brush*>& brushes): brushes(brushes) {
			bounds = brushes[0]->bounds;
			for(const auto& brush: brushes) {
				bounds = merge(bounds, brush->bounds);
			}
		}
		brush* intersect_ray(const glm::vec3& origin, const glm::vec3& direction, float& tClosest) const {
			float t_min, t_max;
			if(!bounds.intersect_ray(origin, direction, t_min, t_max))
				return nullptr;
			brush* hit_brush = nullptr;
			for(const auto& brush: brushes) {
				float t;
				if(brush->intersect_ray(origin, direction, t) && t < tClosest) {
					tClosest = t;
					hit_brush = brush;
				}
			}
			if(left) {
				brush* left_hit = left->intersect_ray(origin, direction, tClosest);
				if(left_hit)
					hit_brush = left_hit;
			}
			if(right) {
				brush* right_hit = right->intersect_ray(origin, direction, tClosest);
				if(right_hit)
					hit_brush = right_hit;
			}
			return hit_brush;
		}
	};


	inline std::unique_ptr<bvh_node> build_bvh(std::vector<brush*>& brushes) {
		if(brushes.size() == 1) {
			return std::make_unique<bvh_node>(brushes);
		}

		// Split along largest axis
		glm::vec3 size = brushes[0]->bounds.max - brushes[0]->bounds.min;
		int axis = (size.x > size.y && size.x > size.z) ? 0 : (size.y > size.z ? 1 : 2);

		std::sort(brushes.begin(), brushes.end(), [axis](const brush* a, const brush* b) {
			return a->bounds.min[axis] < b->bounds.min[axis];
		});

		size_t mid = brushes.size() / 2;
		std::vector<brush*> left_brushes(brushes.begin(), brushes.begin() + mid);
		std::vector<brush*> right_brushes(brushes.begin() + mid, brushes.end());

		auto node = std::make_unique<bvh_node>(brushes);
		node->left = build_bvh(left_brushes);
		node->right = build_bvh(right_brushes);
		return node;
	}
}
#endif