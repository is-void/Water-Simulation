#include "SurfaceWater.h"
#include <cassert>
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include "CircularBuffer.h"


struct Splash {
    float xPos;
    float zPos;
    float time;
    float startingRadius;
    float height;
    float speed;
    float dirX;
    float dirZ;

    Splash() : time((float)glfwGetTime()), xPos(0), zPos(0), startingRadius(0), height(1), speed(0), dirX(0), dirZ(0) {}
    Splash(float x, float z, float t, float strRad, float h, float s, float dx, float dz)
    {
        xPos = x;
        zPos = z;
        time = t;
        startingRadius = strRad;
        height = h;
        speed = s;
        dirX = dx;
        dirZ = dz;
    }
};

unsigned const int MAX_WATER_SPLASHES = 5;

unsigned int VAO;
unsigned int VBO;
unsigned int EBO;
unsigned int splashUBO;
unsigned int waterMap;

CircularBuffer<Splash> waterSources(MAX_WATER_SPLASHES);



void sendSplashData();

SurfaceWater::SurfaceWater(glm::vec3 position, glm::vec<2, int> dimensions, int detail) : Object(position, "./Shaders/water.vts", "./Shaders/water.fgs", "./Shaders/LOD.tcs",  "./Shaders/water.tes")
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
    prepare();


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

void SurfaceWater::prepare()
{

    //Water

    //HeightMap
    glGenTextures(1, &waterMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, waterMap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("./Resources/Textures/waterHeightMap.png", &width, &height, &nrChannels, 0);
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
        std::cout << "Failed to load texture: waterHeightMap.png" << std::endl;
    }
    stbi_image_free(data);

    Object::shader->use();
    Object::shader->setInt("waterMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, waterMap);


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

    
    glGenBuffers(1, &splashUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, splashUBO);
    glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(Splash), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    unsigned int blockIndex = glGetUniformBlockIndex(shader->ID, "SplashData");
    glUniformBlockBinding(shader->ID, blockIndex, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, splashUBO);


    shader->use();
    shader->setVec3("lightColor", glm::vec3(1.0f));
    shader->setVec3("lightDir", glm::vec3(-0.2f, -1.0f, -0.3f));

    Shader::Material waterMat;
    waterMat.ambient = glm::vec3(0.0f, 0.1f, 0.2f);
    waterMat.diffuse = glm::vec3(0.0f, 0.5f, 1.0f);
    waterMat.specular = glm::vec3(0.5f);
    waterMat.shininess = 64.0f;

    shader->setMaterial("material", waterMat);
}

void SurfaceWater::render(Camera* camera, glm::mat4 projection, glm::mat4 view)
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
void sendSplashData()
{
    Splash uploadArray[5];
    for (int i = 0; i < 5; ++i) {
        uploadArray[i] = waterSources.buffer[i];
    }

    glBindBuffer(GL_UNIFORM_BUFFER, splashUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uploadArray), uploadArray);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void SurfaceWater::pushWaterSource(glm::vec3 location)
{
    waterSources.push(Splash(location.x, location.z, (float)glfwGetTime(), 1.0f, 1.0f, 1.0f, 0.0f, 0.0f));
    std::cout << "added" << std::endl;

}
void SurfaceWater::sendData()
{
    shader->use();
    shader->setFloat("Time", (float)glfwGetTime());
    sendSplashData();
}