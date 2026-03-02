#include "Object.h";
#include <vector>
#pragma once
class SurfaceWater : public Object
{
	
	
	struct Point {
		glm::vec3 positionOffset = glm::vec3(0.0f);
		float initialHeight = 0;
		float heightDisplacement = 0;
	};
	
	float maxVarience = 1.0f;
	float stiffness = 0.1f;
	
	public:
		unsigned int vertexCount;
		unsigned int indexCount;
		std::vector<Point> points;
		float* verticies;
		unsigned int* triangles;
		SurfaceWater(glm::vec3 position, glm::vec<2, int> dimensions, int detail);
		float* GenerateVerticies();


};