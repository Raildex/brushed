#ifndef BRUSHED_INCLUDE_BRUSHED_TRANSFORM_HPP
#define BRUSHED_INCLUDE_BRUSHED_TRANSFORM_HPP
#include "glm/common.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
namespace brushed {

	struct transform {
		glm::vec3 pos;
		glm::vec3 euler_rotation;
		glm::vec3 scale;

	public:
		transform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale): pos(pos), euler_rotation(rot), scale(scale) {
		}
		void set_position(glm::vec3 new_pos) {
			pos = new_pos;
		}

		void add_rotation(glm::vec3 delta_rot) {
			euler_rotation += delta_rot;
			euler_rotation = glm::mod(euler_rotation, (float)M_PI * 2);
		}

		void set_rotation(glm::vec3 new_rot) {
			euler_rotation += new_rot;
			euler_rotation = glm::mod(euler_rotation, (float)M_PI * 2);
		}

		void add_scale(glm::vec3 delta_scale) {
			scale += delta_scale;
		}

		void set_scale(glm::vec3 new_scale) {
			scale = new_scale;
		}

		void move_foward(float amount) {
			glm::vec3 dir;

			dir.x = cos(euler_rotation.x) * cos(euler_rotation.y);

			dir.y = sin(euler_rotation.x);

			dir.z = cos(euler_rotation.x) * sin(euler_rotation.y);

			dir = glm::normalize(dir);
			pos += dir * amount;
		}

		void move_right(float amount) {
			glm::vec3 dir;

			dir.x = cos(euler_rotation.x) * cos(euler_rotation.y - glm::radians(90.0f));

			dir.y = sin(euler_rotation.x);

			dir.z = cos(euler_rotation.x) * sin(euler_rotation.y - glm::radians(90.0f));

			dir = glm::normalize(dir);
			pos += dir * amount;
		}

		void move_up(float amount) {
			glm::vec3 dir;

			dir.x = cos(euler_rotation.x - glm::radians(90.0f)) * cos(euler_rotation.y);

			dir.y = sin(euler_rotation.x);

			dir.z = cos(euler_rotation.x) * sin(euler_rotation.y - glm::radians(90.0f));

			dir = glm::normalize(dir);
			pos += dir * amount;
		}

		glm::mat4 to_mat() const {
			return glm::scale(glm::identity<glm::mat4>(), scale)
				* glm::eulerAngleXYZ(euler_rotation.x, euler_rotation.y, euler_rotation.z)
				* glm::translate(glm::identity<glm::mat4>(), pos);
		}
	};
}
#endif