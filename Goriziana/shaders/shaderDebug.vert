#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(){
	vColor = color;
	
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}