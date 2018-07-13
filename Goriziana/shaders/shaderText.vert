#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

//Projection Matrix
uniform mat4 projectionMatrix;

void main(){
	//Moltiplico i vertici solo per la projectionMatrix in modo da gestire facilmente la posizione del testo
	gl_Position = projectionMatrix * vec4(vertex.xy, 0.0, 1.0);
	
	TexCoords = vertex.zw;
}