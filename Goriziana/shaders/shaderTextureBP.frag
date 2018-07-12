#version 330 core

struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D tex;
    vec3 specular;
    float shininess;
};

uniform Light sunLight;

uniform Material material;

// variabile di output dello shader
out vec4 colorFrag;

// normale (in coordinate vista)
in vec3 vNormal;
//vettore da vertice a camera (in coordinate vista)
in vec3 vViewPosition;

// il valore è stato calcolato per-vertex nel vertex shader
in vec2 interp_UV;

//uniform che indica il numero di ripetizioni della texture
uniform float repeat;

// pesi delle componenti
uniform float Kd;
uniform float Ka;
uniform float Ks;

// coefficiente di shininess, passato dall'applicazione
uniform float shininess;



void main(){
	vec3 color;
	
    // applico la ripetizione delle UV e campiono la texture
    vec2 repeated_Uv = mod(interp_UV*repeat, 1.0);
    vec3 surfaceColor = texture(material.tex, repeated_Uv).rgb; 

    // comincio a calcolare la componente ambientale
    vec3 ambient = Ka * sunLight.ambient;
	
    vec3 N = normalize(vNormal);
	
	vec3 L = normalize(-sunLight.direction);

	// calcolo del coefficiente del modello di Lambert
	float lambertian = max(dot(L, N), 0.0);

	// se la componente lambertiana è positiva, procedo al calcolo della componente speculare
	if(lambertian > 0.0) {
		vec3 diffuse = Kd * lambertian * surfaceColor;
		
		// il vettore di vista era stato calcolato nel vertex shader, e il valore era stato già negato per avere il verso dal punto alla camera.
		vec3 V = normalize( vViewPosition );

		// nel modello di Blinn-Phong non utilizzo il vettore di riflessione,
		// ma il vettore posizionato a metà tra direzione di vista e direzione della luce
		vec3 H = normalize(L + V);

		// applico halfdir nel calcolo della componente speculare
		float specAngle = max(dot(N, H), 0.0);
		// applicazione della shininess
		float specularValue = pow(specAngle, shininess);
		
		vec3 specular = Ks * specularValue * material.specular * sunLight.specular;
		// aggiungo le componenti diffusiva e speculari al colore finale 
		color = ambient + diffuse + specular;
	}
    // unisco color e reflectedcolor sulla base di reflectamount
    colorFrag  = vec4(color, 1.0);

}