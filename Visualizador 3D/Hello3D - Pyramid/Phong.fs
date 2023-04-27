//C�digo fonte do Fragment Shader (em GLSL)
#version 450

//Informa��es recebidas do vertex shader
in vec3 finalColor;
in vec3 scaledNormal;
in vec3 fragPos;

//Propriedades do material do objeto
uniform float ka;
uniform float kd;
uniform float ks;
uniform float n;

//Propriedades da fonte de luz
uniform vec3 lightPos;
uniform vec3 lightColor;

//Posi��o da c�mera 
uniform vec3 cameraPos;

//Buffer de sa�da (color buffer)
out vec4 color;

void main()
{
    // Ambient
    vec3 ambient =  lightColor * ka;
    // Diffuse 
    vec3 N = normalize(scaledNormal);
    vec3 L = normalize(lightPos - fragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * kd;
    
    // Specular
    vec3 V = normalize(cameraPos - fragPos);
    vec3 R = normalize (reflect(-L, N));
    vec3 specular = vec3(0.0,0.0,0.0);
        
    vec3 result = (ambient + diffuse) * finalColor;// + specular;

    color = vec4(result, 1.0f);
}
