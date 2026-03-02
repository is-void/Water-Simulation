#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
class Transform {
	public:
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		Transform();
		Transform(glm::vec3 worldPosition, glm::vec3 eulerAngles, glm::vec3 scale);

		glm::mat4 GetModelMatrix();

};