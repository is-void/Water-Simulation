#ifndef SHADER_H
#define SHADER_H

#pragma once
#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {


public:
	unsigned int ID;
	Shader(const char* vertexPath, const char* fragmentPath);
	Shader(const char* vertexPath, const char* fragmentPath, const char* tessControlShader, const char* tessEvalShader);
	struct Material {
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};

	void use();

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec4(const std::string& name, glm::vec4 value) const;
	void setMat4(const std::string& name, glm::mat4 value) const;
	void setMaterial(const std::string& name, Material material) const;
};

#endif#pragma once
