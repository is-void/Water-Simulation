#include "Object.h";
#include <vector>
#pragma once
class Plane : public Object
{
	
	public:
		unsigned int vertexCount;
		
		Plane(const char* vertexShader, const char* tessShader, const char* fragShader, glm::vec3 position, glm::vec<2, int> dimensions, int detail);
		Plane(const char* vertexShader, const char* fragShader, glm::vec3 position, glm::vec<2, int> dimensions, int detail);
		void init(glm::vec3 position, glm::vec<2, int> dimensions, int detail);
		float* GenerateVerticies();
		void render(Camera* camera, glm::mat4 projection, glm::mat4 view) override;


};