#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../WaterSim/Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        std::cout << "for shader located at " << vertexPath << " and " << fragmentPath << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();


    unsigned int vertexID = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentID = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexID, 1, &vShaderCode, NULL);
    glShaderSource(fragmentID, 1, &fShaderCode, NULL);

    int success;
    char infoLog[1024];

    glCompileShader(vertexID);

    glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    glCompileShader(fragmentID);

    glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    ID = glCreateProgram();

    glAttachShader(ID, vertexID);
    glAttachShader(ID, fragmentID);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }


    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* tessControlShader, const char* tessEvalShader) : Shader(vertexPath, fragmentPath)
{
    std::string tessControlCode;
    std::string tessEvalCode;
    std::ifstream tcShaderFile;
    std::ifstream teShaderFile;
    // ensure ifstream objects can throw exceptions:
    tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        tcShaderFile.open(tessControlShader);
        teShaderFile.open(tessEvalShader);
        std::stringstream tcShaderStream, teShaderStream;
        // read file's buffer contents into streams
        tcShaderStream << tcShaderFile.rdbuf();
        teShaderStream << teShaderFile.rdbuf();
        // close file handlers
        tcShaderFile.close();
        teShaderFile.close();
        // convert stream into string
        tessControlCode = tcShaderStream.str();
        tessEvalCode = teShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        std::cout << "for shader located at " << tessControlShader << " and " << tessEvalShader << std::endl;
    }
    const char* tcShaderCode = tessControlCode.c_str();
    const char* teShaderCode = tessEvalCode.c_str();

    // Compile TCS
    unsigned int tcID = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tcID, 1, &tcShaderCode, NULL);
    glCompileShader(tcID);
    int success;
    char infoLog[1024];
    glGetShaderiv(tcID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(tcID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::TESS_CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile TES
    unsigned int teID = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(teID, 1, &teShaderCode, NULL);
    glCompileShader(teID);
    glGetShaderiv(teID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(teID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::TESS_EVALUATION::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Attach and relink program
    glAttachShader(ID, tcID);
    glAttachShader(ID, teID);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::RELINKING_WITH_TESS_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(tcID);
    glDeleteShader(teID);
}


void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetVec3(const std::string& name, glm::vec3 value)
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::SetVec4(const std::string& name, glm::vec4 value)
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));

}




