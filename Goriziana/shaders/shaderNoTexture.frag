/*
13b_blinnphong.frag: Fragment shader per implementazione modello di illuminazione di Blinn-Phong.
   
NB) utilizzare "13_phong.vert" come vertex shader

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano

*/

#version 330 core

// numero di luci nella mia scena
#define NR_LIGHTS 3

// variabile di output dello shader
out vec4 colorFrag;

// vettore di direzioni di incidenza della luce (calcolato nel vertex shader)
in vec3 lightDirs[NR_LIGHTS];
// normale (in coordinate vista)
in vec3 vNormal;
//vettore da vertice a camera (in coordinate vista)
in vec3 vViewPosition;


// il colore della componente ambientale, diffusiva e speculare sono 
// comunicate dall'applicazione
uniform vec3 ambientColor; 
uniform vec3 diffuseColor; 
uniform vec3 specularColor; 
// pesi delle componenti
uniform float Ka;
uniform float Kd;
uniform float Ks;
// parametri per attenuazione
uniform float constant;
uniform float linear;
uniform float quadratic;
// coefficiente di shininess, passato dall'applicazione
uniform float shininess;



void main(){

    // comincio a calcolare la componente ambientale
    vec3 color = Ka*ambientColor;

    vec3 N = normalize(vNormal);
    
	//per tutte le luci nella scena
    for(int i = 0; i < NR_LIGHTS; i++){
		vec3 L = normalize(lightDirs[i].xyz);
		float distanceL = length(L);

		float attenuation = 1.0/(constant + linear*distanceL + quadratic*(distanceL*distanceL));

		// calcolo del coefficiente del modello di Lambert
		float lambertian = max(dot(L,N), 0.0);
		
		// se la componente lambertiana è positiva, procedo al calcolo della componente speculare
		if(lambertian > 0.0) {
		  // il vettore di vista era stato calcolato nel vertex shader, e il valore era stato già negato per avere il verso dal punto alla camera.
		  vec3 V = normalize( vViewPosition );

		  // nel modello di Blinn-Phong non utilizzo il vettore di riflessione,
		  // ma il vettore posizionato a metà tra direzione di vista e direzione della luce
		  vec3 H = normalize(L + V);

		  // applico halfdir nel calcolo della componente speculare
		  float specAngle = max(dot(H, N), 0.0);
		  // applicazione della shininess
		  float specular = pow(specAngle, shininess);

		  // aggiungo le componenti diffusiva e speculari al colore finale 
		  // NB: in questa implementazione la somma dei pesi potrebbe non essere uguale a 1
		  color += vec3( Kd * lambertian * diffuseColor +
						  Ks * specular * specularColor);
						  
		  color*=attenuation;

		}
	}    

    // unisco color e reflectedcolor sulla base di reflectamount
    colorFrag  = vec4( color, 1.0);

}