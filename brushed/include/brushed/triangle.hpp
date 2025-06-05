#ifndef BRUSHED_INCLUDE_BRUSHED_TRIANGLE_HPP
#define BRUSHED_INCLUDE_BRUSHED_TRIANGLE_HPP
#include <glm/glm.hpp>
namespace brushed {

	struct triangle {
		glm::vec3 v0;
		glm::vec3 v1;
		glm::vec3 v2;
	};
}
#endif