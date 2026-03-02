#include "SurfaceWater.h"
#include <cassert>

SurfaceWater::SurfaceWater(glm::vec3 position, glm::vec<2, int> dimensions, int detail) : Object(position, "./Shaders/water.vts", "./Shaders/water.fgs")
{
    int width = dimensions.x * detail;   // number of quads in X
    int depth = dimensions.y * detail;   // number of quads in Z

    int verticesPerRow = width + 1;
    int verticesPerCol = depth + 1;

    vertexCount = verticesPerRow * verticesPerCol;
    indexCount = width * depth * 6;  // 2 triangles per quad, 3 indices each

    float step = 1.0f / detail;

    points.resize(vertexCount);
    triangles = new unsigned int[indexCount];


    float halfWidth = (width * step) * 0.5f;
    float halfDepth = (depth * step) * 0.5f;

    //
    // Generate vertices
    //

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

    //
    // Generate triangle indices
    //
    int triIndex = 0;


    for (int j = 0; j < depth; j++) {
        // ...
    }
    for (int j = 0; j < depth; j++)
    {
        for (int i = 0; i < width; i++)
        {
            unsigned int topLeft = j * verticesPerRow + i;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (j + 1) * verticesPerRow + i;
            unsigned int bottomRight = bottomLeft + 1;

            // First triangle
            triangles[triIndex++] = topLeft;
            triangles[triIndex++] = bottomLeft;
            triangles[triIndex++] = topRight;

            // Second triangle
            triangles[triIndex++] = topRight;
            triangles[triIndex++] = bottomLeft;
            triangles[triIndex++] = bottomRight;
        }
    }

    GenerateVerticies();

}

float* SurfaceWater::GenerateVerticies()
{
    if (verticies !=  nullptr)
    {
        return verticies;
    }
    verticies = new float[vertexCount * 3];
    
    for (int i = 0; i < vertexCount; i++)
    {
        glm::vec3 loc = transform->position + points[i].positionOffset + points[i].heightDisplacement;
        
        verticies[i * 3] = loc.x;
        verticies[i * 3 + 1] = loc.y;
        verticies[i * 3 + 2] = loc.z;
    }
    return verticies;

}
