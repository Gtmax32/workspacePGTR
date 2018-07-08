#version 330 core
layout (location = 0) in vec3 position;

out vec3 vColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(){
    vColor = vec3(1.0, 1.0, 1.0);
	//vColor = color;
	
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}