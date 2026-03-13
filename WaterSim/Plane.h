#include "Object.h";
#include <vector>
#pragma once
class Plane : public Object
{
	
	public:
		unsigned int vertexCount;
		

		Plane(glm::vec3 position, glm::vec<2, int> dimensions, int detail);
		float* GenerateVerticies();
		void render(Camera* camera, glm::mat4 projection, glm::mat4 view) override;


};