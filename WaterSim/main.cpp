#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../WaterSim/Shader.h"
#include "../WaterSim/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "SurfaceWater.h"
#include "CircularBuffer.h"






void sendSplashData(int uboSplashes);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void pushWaterSource(glm::vec3 location);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 800;

float lastTime;
float deltaTime;
bool firstMouse = true;
bool spacePressed = false;

float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;


float lastMouseX = SCREEN_WIDTH / 2.0f;
float lastMouseY = SCREEN_HEIGHT / 2.0f;


const unsigned int MAX_WATER_SPLASHES = 5;

struct Splash {
	float xPos;
	float zPos;
	float time;
	float startingRadius;
	Splash() : time(0), xPos(0), zPos(0), startingRadius(0) {}
	Splash(float x, float z, float t, float strRad)
	{
		xPos = x;
		zPos = z;
		time = t;
		startingRadius = strRad;
	}
};
CircularBuffer<Splash> waterSources(MAX_WATER_SPLASHES);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "I'm GLing it", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window";
		glfwTerminate();
		return -1;

	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	SurfaceWater waterObj = SurfaceWater(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(100.0f, 100.0f), 20);
	const int rez = 20; // This should match the 'detail' argument passed to SurfaceWater




	const char* waterVertShaderSource = "./Shaders/water.vts";
	const char* waterFragShaderSource = "./Shaders/water.fgs";





	unsigned int VAO2;
	unsigned int VBO2;
	unsigned int EBO2;
	

	//Water

	
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);


	glGenBuffers(1, &EBO2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);


	glGenBuffers(1, &VBO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * waterObj.vertexCount * 3, waterObj.GenerateVerticies(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);


	glBindVertexArray(0);

	unsigned int splashUBO;
	glGenBuffers(1, &splashUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, splashUBO);
	glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(Splash), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	unsigned int blockIndex = glGetUniformBlockIndex(waterObj.shader->ID, "SplashData");
	glUniformBlockBinding(waterObj.shader->ID, blockIndex, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, splashUBO);


	unsigned int waterModelMatrixLoc = glGetUniformLocation(waterObj.shader->ID, "model");
	unsigned int waterViewMatrixLoc = glGetUniformLocation(waterObj.shader->ID, "view");
	unsigned int waterProjectionMatrixLoc = glGetUniformLocation(waterObj.shader->ID, "projection");

	glEnable(GL_DEPTH_TEST);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);



	while (!glfwWindowShouldClose(window))
	{

		//input
		processInput(window);

		//rendering
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		/*
		//Model Matrix: Scaling/Rotating/Transforming
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		*/


		//View Matrix/Camera
		glm::mat4 view = camera.GetViewMatrix();


		//Projection Matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);


		
		
		waterObj.shader->use();
		glUniformMatrix4fv(waterViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(waterProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO2);
		

		//Water
		waterObj.shader->setMat4("model", waterObj.transform->GetModelMatrix());
		glDrawArrays(GL_PATCHES, 0, 4 * waterObj.vertexCount);



		//check and call events, swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


		waterObj.shader->setFloat("Time", (float)glfwGetTime());
		sendSplashData(splashUBO);

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

	}
	glfwTerminate();
	return 0;
}


void sendSplashData(int uboSplashes)
{
	Splash uploadArray[5];
	for (int i = 0; i < 5; ++i) {
		uploadArray[i] = waterSources.buffer[i];
	}

	glBindBuffer(GL_UNIFORM_BUFFER, uboSplashes);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uploadArray), uploadArray);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	const float cameraSpeed = 2.5f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (!spacePressed)
		{
			pushWaterSource(glm::vec3(0.0));
			spacePressed = true;
		}
	}
	else
	{
		spacePressed = false;
	}


}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void pushWaterSource(glm::vec3 location)
{
	waterSources.push(Splash(location.x, location.z, (float)glfwGetTime(), 1.0f));
	std::cout << "added" << std::endl;

}
