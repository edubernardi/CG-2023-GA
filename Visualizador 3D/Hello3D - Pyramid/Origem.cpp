/* Hello Triangle - c�digo adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gr�fico/Computa��o Gr�fica - Unisinos
 * Vers�o inicial: 7/4/2017
 * �ltima atualiza��o em 01/03/2023
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

// Prot�tipo da fun��o de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Prot�tipos das fun��es
int setupShader();
int loadOBJ(string filepath, int& nVerts);

// Dimens�es da janela (pode ser alterado em tempo de execu��o)
const GLuint WIDTH = 1920, HEIGHT = 1080;


//C�difo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
};

bool rotateX = false, rotateY = false, rotateZ = false;
glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
float speed = 0.05;

bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;
float yaw = -90.0, pitch = 0.0;

float fov = 45.0;

vector <Vertex> vertices;
vector <GLuint> indices;
vector <glm::vec3> normals;
vector <glm::vec2> texCoord;

// Fun��o MAIN
int main()
{
	// Inicializa��o da GLFW
	glfwInit();

	//Muita aten��o aqui: alguns ambientes n�o aceitam essas configura��es
	//Voc� deve adaptar para a vers�o do OpenGL suportada por sua placa
	//Sugest�o: comente essas linhas de c�digo para desobrir a vers�o e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
	//#ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//#endif

	// Cria��o da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Visualizador 3D", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da fun��o de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	//callback do mouse
	glfwSetCursorPosCallback(window, mouse_callback);

	//callback do scroll
	glfwSetScrollCallback(window, scroll_callback);

	//desabilitando cursor mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLAD: carrega todos os ponteiros d fun��es da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informa��es de vers�o
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimens�es da viewport com as mesmas dimens�es da janela da aplica��o
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	Shader shader("Phong.vs", "Phong.fs");

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	//Definindo a matriz de view (posi��o e orienta��o da c�mera)
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));

	//Definindo a matriz de proje��o perpectiva
	glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");
	glUniformMatrix4fv(projLoc, 1, FALSE, glm::value_ptr(projection));

	glEnable(GL_DEPTH_TEST);

	model = glm::mat4(1);
	glUseProgram(shader.ID);

	//Definindo as propriedades do material 
	shader.setFloat("ka", 0.2);
	shader.setFloat("kd", 0.5);
	shader.setFloat("ks", 0.5);
	shader.setFloat("n", 10);

	//Definindo as propriedades da fonte de luz
	shader.setVec3("lightPos", -2.0f, 100.0f, 2.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);



	

	int nVerts;
	GLuint VAO = loadOBJ("../Pikachu.obj", nVerts);

	glEnable(GL_DEPTH_TEST);

	// Loop da aplica��o - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as fun��es de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(5);
		glPointSize(0);

		float xRotation = 0.0;
		float yRotation = 0.0;
		float zRotation = 0.0;

		if (rotateX)
		{
			xRotation = speed;

		}
		else {
			xRotation = 0.0;
		}
		if (rotateY)
		{
			yRotation = speed;

		}
		else {
			yRotation = 0.0;
		}
		if (rotateZ)
		{
			zRotation = speed;

		}
		else {
			zRotation = 0.0;
		}

		if (rotateX || rotateY || rotateZ) {
			model = glm::rotate(model, speed, glm::vec3(float(xRotation), float(yRotation), float(zRotation)));
		}

		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		glUniformMatrix4fv(projLoc, 1, FALSE, glm::value_ptr(projection));

		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, nVerts);

		// Chamada de desenho - drawcall
		// CONTORNO - GL_LINE_LOOP

		glDrawArrays(GL_POINTS, 0, nVerts);
		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execu��o da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Fun��o de callback de teclado - s� pode ter uma inst�ncia (deve ser est�tica se
// estiver dentro de uma classe) - � chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_P && (action == GLFW_PRESS))
	{
		speed += 0.1;
	}

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		speed -= 0.1;
	}

	cout << "\nSpeed:" << speed;

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		if (rotateX) {
			rotateX = false;
		}
		else {
			rotateX = true;
		}
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		if (rotateY) {
			rotateY = false;
		}
		else {
			rotateY = true;
		}
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		if (rotateZ) {
			rotateZ = false;
		}
		else {
			rotateZ = true;
		}
	}

	if (key == GLFW_KEY_W)
	{
		cameraPos += cameraFront * float(0.1);
	}

	if (key == GLFW_KEY_S)
	{
		cameraPos -= cameraFront * float(0.1);
	}

	if (key == GLFW_KEY_A)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * float(0.1);
	}

	if (key == GLFW_KEY_D)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * float(0.1);
	}

	//vis�o superior
	if (key == GLFW_KEY_1) 
	{
		cameraPos = glm::vec3(0.0, 5.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(0.00108709, -0.999701, 0.02441);
	}

	//vis�o de frente
	if (key == GLFW_KEY_2)
	{
		cameraPos = glm::vec3(0.0, 0.0, 3.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.0235599, -0.00523596, -0.999709);
	}

	//vis�o direita
	if (key == GLFW_KEY_3)
	{
		cameraPos = glm::vec3(3.0, 0.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.99, -0.00523596, 0.062);
	}

	//vis�o traseira
	if (key == GLFW_KEY_4)
	{
		cameraPos = glm::vec3(0.0, 0.0, -3.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.006, -0.007, 0.99);
	}

	//vis�o esquerda
	if (key == GLFW_KEY_5)
	{
		cameraPos = glm::vec3(-3.0, 0.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(0.99, -0.02, -0.03);
	}

	//vis�o de baixo (s� da pra ver o ch�o)
	if (key == GLFW_KEY_6)
	{
		cameraPos = glm::vec3(0.0, -5.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.001, 0.99, -0.26);
	}

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float sensitivity = 0.05;
	// cout << xpos << " " << ypos << endl;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float offsetx = xpos - lastX;
	float offsety = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	offsetx *= sensitivity;
	offsety *= sensitivity;

	pitch += offsety;
	yaw += offsetx;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	/*
	cout << "\nCameraPos x:" << cameraPos.x;
	cout << "\nCameraPos y:" << cameraPos.y;
	cout << "\nCameraPos z:" << cameraPos.z;

	cout << "\nCameraFront x:" << cameraFront.x;
	cout << "\nCameraFront y:" << cameraFront.y;
	cout << "\nCameraFront z:" << cameraFront.z;

	cout << "\nCameraUp x:" << cameraUp.x;
	cout << "\nCameraUp y:" << cameraUp.y;
	cout << "\nCameraUp z:" << cameraUp.z;
	*/
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= yoffset;
}


// Esta fun��o est� bastante harcoded - objetivo � criar os buffers que armazenam a 
// geometria de um tri�ngulo
// Apenas atributo coordenada nos v�rtices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A fun��o retorna o identificador do VAO

int loadOBJ(string filePath, int& nVerts)
{
	ifstream inputFile;
	inputFile.open(filePath);
	vector <GLfloat> vertbuffer;

	if (inputFile.is_open())
	{
		char line[100];
		string sline;


		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;

			string word;
			istringstream ssline(sline);

			ssline >> word;

			if (word == "v")
			{
				Vertex v;
				ssline >> v.position.x >> v.position.y >> v.position.z;
				v.color.r = 1.0; v.color.g = 0.0; v.color.b = 0.0;
				vertices.push_back(v);
			}
			if (word == "vt")
			{
				glm::vec2 vt;
				ssline >> vt.s >> vt.t;
				texCoord.push_back(vt);
			}
			if (word == "vn")
			{
				glm::vec3 vn;
				ssline >> vn.x >> vn.y >> vn.z;
				normals.push_back(vn);
			}
			else if (word == "f")
			{
				string tokens[3];
				for (int i = 0; i < 3; i++)
				{
					ssline >> tokens[i];
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					indices.push_back(index);
					vertbuffer.push_back(vertices[index].position.x);
					vertbuffer.push_back(vertices[index].position.y);
					vertbuffer.push_back(vertices[index].position.z);
					vertbuffer.push_back(vertices[index].color.r);
					vertbuffer.push_back(vertices[index].color.g);
					vertbuffer.push_back(vertices[index].color.b);

					tokens[i] = tokens[i].substr(pos + 1);
					pos = tokens[i].find("/");
					token = tokens[i].substr(0, pos);
					int indexT = atoi(token.c_str()) - 1;

					vertbuffer.push_back(texCoord[indexT].s);
					vertbuffer.push_back(texCoord[indexT].t);

					tokens[i] = tokens[i].substr(pos + 1);
					token = tokens[i].substr(0, pos);
					int indexN = atoi(token.c_str()) - 1;

					vertbuffer.push_back(normals[indexN].x);
					vertbuffer.push_back(normals[indexN].y);
					vertbuffer.push_back(normals[indexN].z);

				}

			}

		}

		inputFile.close();
	}
	else
	{
		cout << "Problema ao encontrar o arquivo " << filePath << endl;
	}


	GLuint VBO, VAO;

	nVerts = vertbuffer.size() / 11;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertbuffer.size() * sizeof(GLfloat), vertbuffer.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	//Pos x, y, z
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Cor r, g,b
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Text coords s, t
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Normal x, y, z
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO
	glBindVertexArray(0);

	return VAO;

}
