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


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Protótipos das funções
int setupShader();
int loadOBJ(string filepath, int& nVerts);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1920, HEIGHT = 1080;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = projection * view * model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

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
	GLuint shaderID = setupShader();

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	//Definindo a matriz de view (posição e orientação da câmera)
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	GLint viewLoc = glGetUniformLocation(shaderID, "view");
	glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));

	//Definindo a matriz de projeção perpectiva
	glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(shaderID, "projection");
	glUniformMatrix4fv(projLoc, 1, FALSE, glm::value_ptr(projection));

	glEnable(GL_DEPTH_TEST);

	model = glm::mat4(1);

	int nVerts;
	GLuint VAO = loadOBJ("../teapot.obj", nVerts);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(5);
		glPointSize(5);

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


//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO

int loadOBJ(string filepath, int& nVerts)
{
	vector <GLfloat> vbuffer;
	ifstream inputFile;
	inputFile.open(filepath.c_str());
	if (inputFile.is_open())
	{
		char line[100];
		string sline;

		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;

			string word;

			istringstream ssline(line);
			ssline >> word;

			if (word == "v")
			{
				Vertex v;

				ssline >> v.position.x >> v.position.y >> v.position.z;
				v.color.r = 1.0;  v.color.g = 0.0;  v.color.b = 0.0;

				vertices.push_back(v);
			}
			if (word == "f")
			{
				string tokens[3];

				ssline >> tokens[0] >> tokens[1] >> tokens[2];

				for (int i = 0; i < 3; i++)
				{
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					cout << vertices[index].position.x << "  " << vertices[index].position.y << " " << vertices[index].position.z << endl;
					vbuffer.push_back(vertices[index].position.x);
					vbuffer.push_back(vertices[index].position.y);
					vbuffer.push_back(vertices[index].position.z);
					vbuffer.push_back(vertices[index].color.r);
					vbuffer.push_back(vertices[index].color.g);
					vbuffer.push_back(vertices[index].color.b);

				}
			}

		}

	}
	else
	{
		cout << "Problema ao encontrar o arquivo " << filepath << endl;
	}
	inputFile.close();

	GLuint VBO, VAO;

	nVerts = vbuffer.size() / 6;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(GLfloat), vbuffer.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	//Pos xyz
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Cor rgb
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);




	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO
	glBindVertexArray(0);

	return VAO;

}
