/*
18_blinnphong_tex_multiplelights.frag: come 16b_blinnphong_tex.frag, ma con supporto a luci multiple

NB1) usare 18_phong_tex_multiplelights.vert come vertex shader
NB2) in questo shader sono considerate solo pointlights, andrebbe modificato in caso di + luci di natura diversa 
NB3) ci sono metodi + efficienti per passare dati multipli agli shader (guardare Uniform Buffer Objects)
NB4) Solo con le ultime versioni di OpenGL, tramite strutture come quelle accennate sopra, è possibile passare un numero dinamico di luci


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

// il valore è stato calcolato per-vertex nel vertex shader
in vec2 interp_UV;

//uniform che indica il numero di ripetizioni della texture
uniform float repeat;

// uniform che indica la texture da utilizzare
uniform sampler2D tex;

// il colore della componente ambientale, diffusiva e speculare sono 
// comunicate dall'applicazione
uniform vec3 ambientColor; 
uniform vec3 specularColor; 
// pesi delle componenti
uniform float Kd;
uniform float Ka;
uniform float Ks;
// parametri per attenuazione
uniform float constant;
uniform float linear;
uniform float quadratic;
// coefficiente di shininess, passato dall'applicazione
uniform float shininess;



void main(){

    // applico la ripetizione delle UV e campiono la texture
    vec2 repeated_Uv = mod(interp_UV*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeated_Uv); 

    // comincio a calcolare la componente ambientale
    vec4 color = vec4(Ka*ambientColor,1.0);

    vec3 N = normalize(vNormal);
    
    //per tutte le luci nella scena
    for(int i = 0; i < NR_LIGHTS; i++)
    {
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
        color += Kd * lambertian * surfaceColor +
                        vec4(Ks * specular * specularColor,1.0);
        color*=attenuation;
      }
    }

    // unisco color e reflectedcolor sulla base di reflectamount
    colorFrag  = color;

}