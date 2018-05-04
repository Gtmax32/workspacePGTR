/*
20_skybox.vert: Vertex shader per la visualizzazione della cube map come environment map

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/


#version 330 core
// posizione vertice in coordinate mondo del cubo che rappresenta il mondo
layout (location = 0) in vec3 position;

// coordinate texture per il campionamento della environment map (sono 3 coordinate visto stiamo campionando su 3 dimensioni)
out vec3 interp_UVW;


// matrice di vista
uniform mat4 viewMatrix;
// matrice di proiezione
uniform mat4 projectionMatrix;

void main()
{
	// in questo caso non uso le UV del modello, ma uso come coordinate texture 3D la posizione del vertice,
	// in modo da avere la mappatura 1:1 tra la cube map e il cubo usato "come mondo"
	interp_UVW = position;

	// applica trasformazioni al vertice
    vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0);
    // Metto la Z dei punti proiettati a 1.0: questo significa che al momento del test di profondità
    // i frammenti della environment map avranno profondità massima (vedere commenti nel codice della applicazione principale)
    gl_Position = pos.xyww;
}  