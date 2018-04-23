#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out VS_OUT {
    vec3 color;
} vs_out;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(){
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
	
	vs_out.color = color;
    
}