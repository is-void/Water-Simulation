#pragma once
#include <glm/ext/vector_float3.hpp>
#include "Transform.h"
#include "Shader.h"
#include "Camera.h"
#include <vector>
class Object
{

	public:
		Shader* shader;
		Transform* transform;
		float* vertices = NULL;
		std::vector<unsigned int> indices;
		Object(glm::vec3 position, const char* vertSource, const char* fragSource);
		Object(glm::vec3 position, const char* vertSource, const char* fragSource, const char* tcsSource, const char* tesSource);
		virtual void render(Camera* camera, glm::mat4 projection, glm::mat4 view){}

};