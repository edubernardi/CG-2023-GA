#pragma once

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

class Mesh
{
public:
	Mesh() {}
	~Mesh() {}
	void initialize(GLuint VAO, int nVertices, Shader* shader, glm::vec3 position = glm::vec3(0.0f), glm::vec3 color = glm::vec3(0.0, 0.0, 1.0), glm::vec3 scale = glm::vec3(1), float angle = 0.0, glm::vec3 axis = glm::vec3(0.0, 0.0, 1.0));
	void update();
	void draw();
	GLuint VAO;
	int nVertices;
	glm::vec3 position;
	float angle;
	glm::vec3 axis;
	glm::vec3 scale;
	glm::vec3 color;
	Shader* shader;

};