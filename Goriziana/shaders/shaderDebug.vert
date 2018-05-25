#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(){
    //vs_out.color = vec3(1.0f, 1.0f, 0.0f);
	vColor = color;
	
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}