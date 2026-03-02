#include "Transform.h"
#include <glm/ext/matrix_transform.hpp>

Transform::Transform()
{
	position = glm::vec3(0.0f);
	rotation = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
}

Transform::Transform(glm::vec3 worldPosition, glm::vec3 eulerAngles, glm::vec3 scale)
{
	position = worldPosition;
	rotation = eulerAngles;
	this->scale = scale;

}

glm::mat4 Transform::GetModelMatrix()
{
	glm::vec3 euler = glm::radians(rotation);

	glm::mat4 model = glm::mat4(1.0f);
	
	model = glm::scale(model, scale);
	model = glm::rotate(model, euler.z, glm::vec3(0, 0, 1)); // Roll
	model = glm::rotate(model, euler.y, glm::vec3(0, 1, 0)); // Yaw
	model = glm::rotate(model, euler.x, glm::vec3(1, 0, 0)); // Pitch

	model = glm::translate(model, position);

	return model;
}
