#version 330 core
layout (location = 0) in vec3 position;

out vec3 TexCoord;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){
	TexCoord = position;

    vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0);
	
    gl_Position = pos.xyww;
}  