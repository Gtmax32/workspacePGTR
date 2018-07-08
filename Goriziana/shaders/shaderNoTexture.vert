/*
13_phong.vert: Vertex shader per il modello di illuminazione di Phong e Blinn-Phong.

Applica le trasformazioni ai vertici, calcola il vettore di vista il vettore di incidenza
della luce a partire dalla posizione della luce,  
Attenzione: lo shader rappresenta un caso semplificato, con una sola luce puntiforme.
In caso di più luci, bisogna effettuare un ciclo for sommando il contributo di ogni luce.
In caso di luci di tipo diverso, va variato il calcolo (per es: una luce direzionale è definita dalla 
direzione dell'incidenza della luce, quindi lightDir è già dato e non va calcolato come in questo caso con
una luce puntiforme).

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/

#version 330 core

// numero di luci nella mia scena
#define NR_LIGHTS 1

// posizione vertice in coordinate mondo
layout (location = 0) in vec3 position;
// normale vertice
layout (location = 1) in vec3 normal;

// matrice di modellazione
uniform mat4 modelMatrix;
// matrice di vista
uniform mat4 viewMatrix;
// matrice di proiezione
uniform mat4 projectionMatrix;

// matrice di trasformazione delle normali (= trasposta dell'inversa della model-view)
uniform mat3 normalMatrix;

// vettore di direzioni di incidenza della luce (passato da applicazione)
uniform vec3 light;

// direzioni di incidenza della luce (in coordinate vista)
out vec3 lightDir;

// normale (in coordinate vista)
out vec3 vNormal;

// oltre al calcolo di normale e posizione in coordinate vista, deve calcolare 
// anche il vettore di riflessione per il riflesso speculare.
// Per fare questo calcolo nel fragment shader, ho bisogno di passare la direzione di vista in coordinate vista,
// che verrà interpolata dal rasterizzatore per ogni frammento
out vec3 vViewPosition;


void main(){

	// posizione vertice in coordinate ModelView (vedere ultima riga per il calcolo finale della posizione in coordinate camera)
	vec4 mvPosition = viewMatrix * modelMatrix * vec4( position, 1.0 );

	// calcolo della direzione di vista, negata per avere il verso dal vertice alla camera
	vViewPosition = -mvPosition.xyz;
	// trasformazione coordinate normali in coordinate vista 
	vNormal = normalize( normalMatrix * normal );
	
	// calcolo del vettore di incidenza della luce (per tutte le luci)
	/*for (int i=0;i<NR_LIGHTS;i++){
		// calcolo del vettore di incidenza della luce.
		vec4 lightPos = viewMatrix  * vec4(lights[i], 1.0);
		lightDirs[i] = lightPos.xyz - mvPosition.xyz;
	}*/
	
	vec4 lightPos = viewMatrix  * vec4(light, 1.0);
	lightDir = lightPos.xyz - mvPosition.xyz;
	
	// calcolo posizione vertici in coordinate vista
	gl_Position = projectionMatrix * mvPosition;

}