#include "Object.h"

Object::Object(glm::vec3 position, const char* vertSource, const char* fragSource)
{
	shader = new Shader(vertSource, fragSource);
	transform = new Transform(position, glm::vec3(0.0f), glm::vec3(1));

}
