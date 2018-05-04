/*
20_skybox.frag: Fragment shader per la visualizzazione della cube map come environment map

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/


#version 330 core

// variabile di output dello shader
out vec4 colorFrag;

// il valore è stato calcolato per-vertex nel vertex shader
in vec3 interp_UVW;

// uniform che indica la texture da utilizzare
uniform samplerCube tCube;

void main()
{    
	// semplicemente campiona la cube map
    colorFrag = texture(tCube, interp_UVW);
}