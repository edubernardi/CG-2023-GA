// C�digo fonte do Vertex Shader (em GLSL)
#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texc;
layout (location = 3) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 inputColor;

out vec3 finalColor;
out vec3 scaledNormal;
out vec3 fragPos;

void main()
{
	//...pode ter mais linhas de c�digo aqui!
	gl_Position = projection * view * model * vec4(position, 1.0);
	finalColor = inputColor;
	//Vetor normal escalada
	scaledNormal = normal; // mat3(transpose(inverse(model))) * normal;
	//Posi��o do v�rtice com a transforma��o do objeto 
	fragPos = vec3(model * vec4(position, 1.0));
}
