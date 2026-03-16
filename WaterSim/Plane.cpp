#include "Plane.h"
#include <cassert>

int width, depth;
float step;
Plane::Plane(const char* vertexShader, const char* tessShader, const char* fragShader, glm::vec3 position, glm::vec<2, int> dimensions, int detail) : Object(position, vertexShader, fragShader, "./Shaders/LOD.tcs", tessShader)
{
    init(position, dimensions, detail);
}

Plane::Plane(const char* vertexShader, const char* fragShader, glm::vec3 position, glm::vec<2, int> dimensions, int detail) : Object(position, vertexShader, fragShader)
{
    init(position, dimensions, detail);
}

void Plane::init(glm::vec3 position, glm::vec<2, int> dimensions, int detail) 
{
    width = dimensions.x * detail;   // number of quads in X
    depth = dimensions.y * detail;   // number of quads in Z
    step = 1.0f / detail;

    int verticesPerRow = width;
    int verticesPerCol = depth;

    float step = 1.0f / detail;


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


    
    GenerateVerticies();


}

float* Plane::GenerateVerticies()
{
    int verticesPerRow = width;
    int verticesPerCol = depth;
    float halfWidth = (width * step) * 0.5f;
    float halfDepth = (depth * step) * 0.5f;

    if (vertices != nullptr)
    {
        return vertices;
    }
    vertices = new float[verticesPerRow * verticesPerCol * 3];

    int k = 0;

    
    for (int j = 0; j < verticesPerCol; j++)
    {
        for (int i = 0; i < verticesPerRow; i++)
        {
            float xPos = i * step;
            float zPos = j * step;

            vertices[k * 3] = transform->position.x - halfWidth + xPos;
            vertices[k * 3 + 1] = transform->position.y;
            vertices[k * 3 + 2] = transform->position.z - halfDepth + zPos;
           
        }
        k++;
    }

    return vertices;

}


void Plane::render(Camera* camera, glm::mat4 projection, glm::mat4 view)
{
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setMat4("model", transform->GetModelMatrix());
    shader->setVec3("camPos", camera->Position);
    glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, 0);
}