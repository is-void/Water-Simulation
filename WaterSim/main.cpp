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
#include "Plane.h"

Shader createSkybox(SurfaceWater waterObj);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateDynamicCubeMaps(SurfaceWater* waterObj, Shader* skyboxShader, Plane& tilePlane, unsigned int skyboxVAO, unsigned int reflectionCube, unsigned int refractionCube, unsigned int cubeFBO, unsigned int* depthRBOs);
void processInput(GLFWwindow* window, SurfaceWater* water);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadCubeMap(std::vector<std::string>);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 800;
const unsigned int REFLECTION_RES = 1000;
float lastTime;
float deltaTime;
bool firstMouse = true;
bool spacePressed = false;

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

float lastMouseX = SCREEN_WIDTH / 2.0f;
float lastMouseY = SCREEN_HEIGHT / 2.0f;

unsigned int skyboxVAO, skyboxVBO;
unsigned int skyboxTextures;



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

	SurfaceWater waterObj = SurfaceWater(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(50, 50), 1);
	Plane tilePlane = Plane("./Shaders/plane.vts", "./Shaders/plane.tes", "./Shaders/plane.fgs", glm::vec3(0.0f, -0.2f, 0.0f), glm::vec2(50, 50), 1);
	waterObj.prepare();
	tilePlane.prepare();

	Shader skyboxShader = createSkybox(waterObj);
	unsigned int cubeFBO, dynamicReflectionCube, dynamicRefractionCube;
	unsigned int depthRBOs[2];
	glGenRenderbuffers(2, depthRBOs);
	for (int i = 0; i < 2; ++i) {
		glBindRenderbuffer(GL_RENDERBUFFER, depthRBOs[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, REFLECTION_RES, REFLECTION_RES);
	}
	glGenFramebuffers(1, &cubeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
	// Attach reflection depth by default
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBOs[0]);

	glGenTextures(1, &dynamicReflectionCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicReflectionCube);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, REFLECTION_RES, REFLECTION_RES, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &dynamicRefractionCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicRefractionCube);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, REFLECTION_RES, REFLECTION_RES, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicReflectionCube, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); 
	glFrontFace(GL_CCW);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	std::vector<std::reference_wrapper<Object>> renderObjs;
	renderObjs.push_back(std::ref(tilePlane));

	while (!glfwWindowShouldClose(window))
	{
		//Reflections
		updateDynamicCubeMaps(&waterObj, &skyboxShader, tilePlane, skyboxVAO, dynamicReflectionCube, dynamicRefractionCube, cubeFBO, depthRBOs);



		//input
		processInput(window, &waterObj);

		//rendering
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//View Matrix/Camera
		glm::mat4 view = camera.GetViewMatrix();

		//Projection Matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.05f, 100.0f);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
		
		//Water
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicReflectionCube);
		waterObj.shader->use();
		waterObj.shader->setInt("reflectionMap", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicRefractionCube);
		waterObj.shader->setInt("refractionMap", 2);

		waterObj.render(&camera, projection, view);

		//Plane
		tilePlane.render(&camera, projection, view);

		//Skybox
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextures);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		//check and call events, swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		waterObj.sendData();

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

	}
	glfwTerminate();
	return 0;
}
void updateDynamicCubeMaps(SurfaceWater* waterObj, Shader* skyboxShader, Plane& tilePlane, unsigned int skyboxVAO, unsigned int reflectionCube, unsigned int refractionCube, unsigned int cubeFBO, unsigned int* depthRBOs)
{
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 100.0f);

	glm::vec3 reflectionCenter = glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z);
	glm::vec3 refractionCenter = glm::vec3(camera.Position.x, camera.Position.y+0.5, camera.Position.z);

	auto makeViews = [](glm::vec3 center) {
		return std::vector<glm::mat4>{
				glm::lookAt(center, center + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
				glm::lookAt(center, center + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
				glm::lookAt(center, center + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
				glm::lookAt(center, center + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
				glm::lookAt(center, center + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
				glm::lookAt(center, center + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)),
		};
		};

	std::vector < glm::mat4> reflectionViews = makeViews(reflectionCenter);
	std::vector < glm::mat4> refractionViews = makeViews(refractionCenter);

	GLint oldViewport[4];
	glGetIntegerv(GL_VIEWPORT, oldViewport);

	glViewport(0, 0, REFLECTION_RES, REFLECTION_RES);
	glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearColor(0.2f, 0.3f, 0.2f, 1.0f);

	unsigned int cubeMaps[2] = { reflectionCube, refractionCube };

	for (int cm = 0; cm < 2; ++cm)
	{
		// Re-attach the correct depth buffer for this cubemap
		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, depthRBOs[cm]);

		bool isReflection = (cubeMaps[cm] == reflectionCube);
		std::vector<glm::mat4>& views = isReflection ? reflectionViews : refractionViews;
		glm::vec3 center = isReflection ? reflectionCenter : refractionCenter;

		for (int i = 0; i < 6; ++i)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMaps[cm], 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "FBO incomplete for face " << i << std::endl;
				continue;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Camera cam = camera;
			cam.Position = center;

			if (isReflection)
			{
				glDisable(GL_CULL_FACE);
				glDepthFunc(GL_LEQUAL);
				skyboxShader->use();
				skyboxShader->setMat4("view", glm::mat4(glm::mat3(views[i])));
				skyboxShader->setMat4("projection", proj);
				glBindVertexArray(skyboxVAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextures);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
				glEnable(GL_CULL_FACE);
				glDepthFunc(GL_LESS);
				waterObj->render(&cam, proj, views[i]);
			}
			else
			{
				glDisable(GL_CULL_FACE);
				glDepthFunc(GL_LEQUAL);
				skyboxShader->use();
				skyboxShader->setMat4("view", glm::mat4(glm::mat3(views[i])));
				skyboxShader->setMat4("projection", proj);
				glBindVertexArray(skyboxVAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextures);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
				glEnable(GL_CULL_FACE);
				glDepthFunc(GL_LESS);
				tilePlane.render(&cam, proj, views[i]);
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
}

Shader createSkybox(SurfaceWater waterObj)
{
	std::vector<std::string> faces = {
		"./Resources/Textures/Skybox/right.png",
		"./Resources/Textures/Skybox/left.png",
		"./Resources/Textures/Skybox/top.png",
		"./Resources/Textures/Skybox/bottom.png",
		"./Resources/Textures/Skybox/front.png",
		"./Resources/Textures/Skybox/back.png"
	};
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	Shader skyboxShader("./Shaders/skybox.vts", "./Shaders/skybox.fgs");
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	skyboxTextures = loadCubeMap(faces);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	return skyboxShader;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, SurfaceWater* water)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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
			water->pushWaterSource(glm::vec3(0.0));
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
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadCubeMap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
