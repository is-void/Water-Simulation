#include "Plane.h"
#include <cassert>
#include "stb_image.h"

namespace {

    int width, depth;
    float step;

    unsigned int VAO, VBO;
    unsigned int EBO;
    unsigned int tileDiffuse, tileNormal, tileSpecular;
}


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
    vertexCount = verticesPerRow * verticesPerCol * 3;
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


void Plane::loadTexture(std::string path, unsigned int index, unsigned int* location)
{
    glGenTextures(1, location);

    std::string fileName = path.substr(path.rfind('/') + 1);

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, *location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        // Ensure proper row alignment for 3-channel images
        if (nrChannels == 3)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLenum format = GL_RGBA;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        // Use matching internal/format types
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // restore default alignment (optional)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }
    else
    {
        std::cout << "Failed to load texture: " << fileName << std::endl;
    }
    stbi_image_free(data);
}
void Plane::prepare()
{
    //Textures
    loadTexture("./Resources/Textures/Tiles/FloorTiles/diffuse.png", 0, &tileDiffuse);
    loadTexture("./Resources/Textures/Tiles/FloorTiles/normal.png", 1, &tileNormal);
    loadTexture("./Resources/Textures/Tiles/FloorTiles/specular.png", 2, &tileSpecular);
    
    //Shaders
    Object::shader->use();
    Object::shader->setInt("diffuseMap", 0);
    Object::shader->setInt("normalMap", 1);
    Object::shader->setInt("specularMap", 2);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);


    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, GenerateVerticies(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);


}

void Plane::render(Camera* camera, glm::mat4 projection, glm::mat4 view)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setMat4("model", transform->GetModelMatrix());
    shader->setVec3("camPos", camera->Position);

    glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, 0);
}
