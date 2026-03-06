#pragma once
#include <glm/ext/vector_float3.hpp>
#include "Transform.h"
#include "Shader.h"
class Object
{

	public:
		Shader* shader;
		Transform* transform;
		Object(glm::vec3 position, const char* vertSource, const char* fragSource);
		Object(glm::vec3 position, const char* vertSource, const char* fragSource, const char* tcsSource, const char* tesSource);

};