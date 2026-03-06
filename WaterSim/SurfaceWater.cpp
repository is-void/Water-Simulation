#include "SurfaceWater.h"
#include <cassert>

SurfaceWater::SurfaceWater(glm::vec3 position, glm::vec<2, int> dimensions, int detail) : Object(position, "./Shaders/water.vts", "./Shaders/water.fgs", "./Shaders/water.tcs",  "./Shaders/water.tes")
{
    int width = dimensions.x * detail;   // number of quads in X
    int depth = dimensions.y * detail;   // number of quads in Z

    int verticesPerRow = width;
    int verticesPerCol = depth;

    vertexCount = verticesPerRow * verticesPerCol;

    float step = 1.0f / detail;

    points.resize(vertexCount);

    
    float halfWidth = (width * step) * 0.5f;
    float halfDepth = (depth * step) * 0.5f;

    //
    // Generate vertices
    //
    indices.reserve((verticesPerRow - 1) * (verticesPerCol - 1) * 4);
    for (int z = 0; z < verticesPerCol - 1; ++z)
    {
        for (int x = 0; x < verticesPerRow - 1; ++x)
        {
            unsigned int topLeft = z * verticesPerRow + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * verticesPerRow + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }


    for (int j = 0; j < verticesPerCol; j++)
    {
        for (int i = 0; i < verticesPerRow; i++)
        {
            float xPos = i * step;
            float zPos = j * step;

            Point p;
            p.positionOffset = glm::vec3(
                position.x - halfWidth + xPos,
                position.y,
                position.z - halfDepth + zPos
            );

            points[j * verticesPerRow + i] = p;
        }
    }
    GenerateVerticies();


}

float* SurfaceWater::GenerateVerticies()
{
    if (vertices !=  nullptr)
    {
        return vertices;
    }
    vertices = new float[vertexCount * 3];
    
    for (int i = 0; i < vertexCount; i++)
    {
        glm::vec3 loc = transform->position + points[i].positionOffset + points[i].heightDisplacement;
        
        vertices[i * 3] = loc.x;
        vertices[i * 3 + 1] = loc.y;
        vertices[i * 3 + 2] = loc.z;
    }
    return vertices;

}
