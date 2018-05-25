#version 330 core
in vec3 vColor;

out vec4 colorFrag;

void main(){
    colorFrag = vec4(vColor, 1.0);
}