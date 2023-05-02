/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 01/03/2023
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
#include "Mesh.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

vector <string> readModels();

// Protótipos das funções
int loadOBJ(string filepath, int& nVerts);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1200, HEIGHT = 1200;


//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
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
bool translateF = false, translateG = false, translateH = false;

glm::vec3 cameraPos = glm::vec3(0.0, 2.0, 8.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

float speed = 0.05;
bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;
float yaw = -90.0, pitch = 0.0;
float fov = 45.0;
//int opcao[3];


vector <Vertex> vertices;
vector <GLuint> indices;
vector <glm::vec3> normals;
vector <glm::vec2> texCoord;
vector <Mesh> models;

int selected = 0;


// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
	//#ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Visualizador 3D", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	//callback do mouse
	glfwSetCursorPosCallback(window, mouse_callback);

	//callback do scroll
	glfwSetScrollCallback(window, scroll_callback);

	//desabilitando cursor mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
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

	//Definindo a matriz de view (posição e orientação da câmera)
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));

	//Definindo a matriz de projeção perpectiva
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
	shader.setFloat("q", 100);
	shader.setFloat("n", 0.2);

	//Definindo as propriedades da fonte de luz
	shader.setVec3("lightPos", 10, 5, 0);
	shader.setVec3("lightColor", 5.0f, 5.0f, 5.0f);

	vector <string> modelNames = readModels();
	GLuint VAO;
	for (int i = 0; i < modelNames.size(); i++) {
		int nVerts;
		VAO = loadOBJ("../" + modelNames[i], nVerts);
		if (VAO != -1) {
			Mesh mesh;
			mesh.initialize(VAO, nVerts, &shader, glm::vec3(3.0 * i, 0, 0.0), glm::vec3(0.46, 0.38, 0.16));
			models.push_back(mesh);
		}
	}

	glEnable(GL_DEPTH_TEST);

	float light_y = -10;
	float light_x = -10;

	

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(5);
		glPointSize(0);


		//giro da luz
		if (light_x > 10) {
			speed *= -1;
		}
		else if (light_x < -10) {
			speed *= -1;
		}

		light_x += speed;
		light_y = sqrt(-(light_x * light_x) + 100);

		if (speed < 0) {
			light_y *= -1;
		}

		shader.setVec3("lightPos", light_x, light_y, 0);


		float angle = (GLfloat)glfwGetTime() * 2;



		/*
		int i = 0;

		while (i <= 2) {
			if (opcao[i] == 7) {

				//Translação
				if (translateF) {

					desenho[0].initialize(VAO, nVerts, &shader, glm::vec3(1.0, 0.0, 0.0));
					i++;
				}
				else if (translateG)
				{
					desenho[0].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 1.0, 0.0));
					i++;
				}
				else if (translateH)
				{
					desenho[0].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 1.0));
					i++;
				}

				//Rotação
				if (rotateX) {

					desenho[0].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0), angle, glm::vec3(1.0f, 0.0f, 0.0f));
					i++;
				}
				else if (rotateY)
				{
					desenho[0].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0), angle, glm::vec3(0.0f, 1.0f, 0.0f));
					i++;
				}
				else if (rotateZ)
				{
					desenho[0].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0), angle, glm::vec3(0.0f, 0.0f, 1.0f));
					i++;
				}
			}

			else if (opcao[i] == 8) {

				//Translação
				if (translateF)
				{
					desenho[1].initialize(VAO, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0));
					i++;
				}

				else if (translateG)
				{
					desenho[1].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 3.0, 0.0));
					i++;
				}

				else if (translateH)
				{
					desenho[1].initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 3.0));
					i++;
				}


				//Rotação
				if (rotateX)
				{
					desenho[1].initialize(VAO, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0), angle, glm::vec3(1.0f, 0.0f, 0.0f));
					i++;
				}

				else if (rotateY)
				{
					desenho[1].initialize(VAO, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0), angle, glm::vec3(0.0f, 1.0f, 0.0f));
					i++;
				}

				else if (rotateZ)
				{
					desenho[1].initialize(VAO, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0), angle, glm::vec3(0.0f, 0.0f, 1.0f));
					i++;
				}
			}
			i++;
		}



		*/
		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));


		glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		glUniformMatrix4fv(projLoc, 1, FALSE, glm::value_ptr(projection));

		

		// Chamada de desenho - drawcall
		for (int i = 0; i < models.size(); i++) {
			if (i == selected) {
				models[i].color = glm::vec3(0.1, 0.1, 0.3);
			}
			else {
				models[i].color = glm::vec3(0.46, 0.38, 0.16);
			}
			models[i].update();
			models[i].draw();
			i++;
		}
		
		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	/*

	if (key == GLFW_KEY_P && (action == GLFW_PRESS))
	{
		speed += 0.1;
	}

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		speed -= 0.1;
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}


	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		translateF = true;
		translateG = false;
		translateH = false;
	}

	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		translateF = false;
		translateG = true;
		translateH = false;
	}

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		translateF = false;
		translateG = false;
		translateH = true;
	}
	*/

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
	/*
	//visão superior
	if (key == GLFW_KEY_1)
	{
		cameraPos = glm::vec3(0.0, 5.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(0.00108709, -0.999701, 0.02441);
	}

	//visão de frente
	if (key == GLFW_KEY_2)
	{
		cameraPos = glm::vec3(0.0, 0.0, 3.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.0235599, -0.00523596, -0.999709);
	}

	//visão direita
	if (key == GLFW_KEY_3)
	{
		cameraPos = glm::vec3(3.0, 0.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.99, -0.00523596, 0.062);
	}

	//visão traseira
	if (key == GLFW_KEY_4)
	{
		cameraPos = glm::vec3(0.0, 0.0, -3.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.006, -0.007, 0.99);
	}

	//visão esquerda
	if (key == GLFW_KEY_5)
	{
		cameraPos = glm::vec3(-3.0, 0.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(0.99, -0.02, -0.03);
	}

	//visão de baixo (só da pra ver o chão)
	if (key == GLFW_KEY_6)
	{
		cameraPos = glm::vec3(0.0, -5.0, 0.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
		cameraFront = glm::vec3(-0.001, 0.99, -0.26);
	}*/


	//Ecolha de desenho
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		selected -= 2;
		if (selected < 0) {
			selected = models.size() - 1;
		}
		cout << "Modelo selecionado : " << selected << "⧵n";
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		selected += 2;
		if (selected > (models.size() - 1)) {
			selected = 0;
		}
		cout << "Modelo selecionado : " << selected << "⧵n";
	}

	//translate
	if (key == GLFW_KEY_UP)
	{
		models[selected].position.z -= 0.1;
	}

	if (key == GLFW_KEY_DOWN)
	{
		models[selected].position.z += 0.1;
	}

	if (key == GLFW_KEY_LEFT)
	{
		models[selected].position.x -= 0.1;
	}

	if (key == GLFW_KEY_RIGHT)
	{
		models[selected].position.x += 0.1;
	}

	if (key == GLFW_KEY_RIGHT_SHIFT)
	{
		models[selected].position.y += 0.1;
	}

	if (key == GLFW_KEY_RIGHT_CONTROL)
	{
		models[selected].position.y -= 0.1;
	}

	//scale
	if (key == GLFW_KEY_O)
	{
		models[selected].scale.x -= 0.1;
		models[selected].scale.y -= 0.1;
		models[selected].scale.z -= 0.1;
	}
	if (key == GLFW_KEY_P)
	{
		models[selected].scale.x += 0.1;
		models[selected].scale.y += 0.1;
		models[selected].scale.z += 0.1;
	}

	//rotation
	if (key == GLFW_KEY_F)
	{
		models[selected].axis = glm::vec3(0.0, 0.0, 1.0);
		models[selected].angle += 0.5;
	}
	if (key == GLFW_KEY_G)
	{
		models[selected].axis = glm::vec3(0.0, 0.0, 1.0);
		models[selected].angle -= 0.5;
	}
	if (key == GLFW_KEY_H)
	{
		models[selected].axis = glm::vec3(0.0, 1.0, 0.0);
		models[selected].angle += 0.5;
	}
	if (key == GLFW_KEY_J)
	{
		models[selected].axis = glm::vec3(0.0, 1.0, 0.0);
		models[selected].angle -= 0.5;
	}
	if (key == GLFW_KEY_K)
	{
		models[selected].axis = glm::vec3(1.0, 0.0, 0.0);
		models[selected].angle += 0.5;
	}
	if (key == GLFW_KEY_L)
	{
		models[selected].axis = glm::vec3(1.0, 0.0, 0.0);
		models[selected].angle -= 0.5;
	}

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float sensitivity = 0.05;

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
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= yoffset;
}


// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO

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
				v.color.r = 0.46; v.color.g = 0.38; v.color.b = 0.16;
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
		return -1;
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

vector <string> readModels() {
	vector <string> modelNames;
	int howManyModels;
	cout << "Quantos modelos voce quer carregar?";
	cin >> howManyModels;
	
	for (int i = 0; i < howManyModels; i++) {
		string modelName;
		cout << "Qual eh o modelo #" << i << " ?";
		cin >> modelName;
		modelNames.push_back(modelName);
	}
	return modelNames;
}