#version 330 core

//Numero di directional light nella scena
#define NR_LIGHTS 2

//Posizione vertice in coordinate mondo
layout (location = 0) in vec3 position;
//Normale al vertice
layout (location = 1) in vec3 normal;

//Model matrix
uniform mat4 modelMatrix;
//View matrix
uniform mat4 viewMatrix;
//Projection matrix
uniform mat4 projectionMatrix;

//Matrice di trasformazione delle normali
uniform mat3 normalMatrix;

//Vettori di incidenza delle diverse directional light
uniform vec3 lightVectors[NR_LIGHTS];

//Vettori di incidenza in coordinate vista
out vec3 lightDirs[NR_LIGHTS];

//Vettori normali in coordinate vista
out vec3 vNormal;

//Vettore da vertice a camera
out vec3 vViewPosition;


void main(){
	//Calcolo la posizione del vertice in coordinate ModelView
	vec4 mvPosition = viewMatrix * modelMatrix * vec4( position, 1.0 );

	//Calcolo la direzione di vista, negata per avere il verso dal vertice alla camera
	vViewPosition = -mvPosition.xyz;

	//Applico le trasformazioni alle coordinate dei vettori normali
	vNormal = normalize( normalMatrix * normal );

	//Calcolo il vettore di incidenza delle directional light
	for (int i = 0; i < NR_LIGHTS; i++){
		lightDirs[i] = vec3(viewMatrix  * vec4(lightVectors[i], 0.0));
	}	

	//Applico la Projection Matrix alla posizione del vertice
	gl_Position = projectionMatrix * mvPosition;
}