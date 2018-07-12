#version 330 core

//Numero di directional light nella scena
#define NR_LIGHTS 2

//Costante PI
const float PI = 3.14159;

//Variabile di output
out vec4 colorFrag;

//Vettori di incidenza delle directional light
in vec3 lightDirs[NR_LIGHTS];
//Vettore normale, calcolato nel vertex
in vec3 vNormal;
//Vettore da vertice a camera
in vec3 vViewPosition;

//Vettore dal fragment alla camera in coordinate vista
in vec2 interp_UV;

//Numero di ripetizioni della texture
uniform float repeat;

//Sampler della texture
uniform sampler2D tex;

uniform float m; //Rugosità superficie
uniform float F0; //Fresnel Reflectance
uniform float Kd; //Componente diffusiva della riflettanza

void main(){
	//Determino la texture da applicare mediante gli UVs ed il numero di ripetizioni
    vec2 repeated_Uv = mod(interp_UV*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeated_Uv);

    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
	
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	
	for (int i = 0; i < NR_LIGHTS; i++){
		//Normalizzo il vettore di incidenza della directional light i
		vec3 L = normalize(lightDirs[i].xyz);

		//Calcolo il coefficiente di Lambert
		float lambertian = max(dot(L,N), 0.0);

		float specular = 0.0;

		//Se il coefficiente di Lambert è positivo, posso calcolare la componente speculare
		if(lambertian > 0.0){
			//Normalizzo il vettore di vista
			vec3 V = normalize( vViewPosition );

			//Calcolo Half Vector
			vec3 H = normalize(L + V);

			//Calcolo il coseno tra i vettori ed i parametri che mi serviranno per calcolare le singole componenti
			float NdotH = max(dot(N, H), 0.0);
			float NdotV = max(dot(N, V), 0.0);
			float VdotH = max(dot(V, H), 0.0);
			float mSquared = m * m;

			//Calcolo il fattore geometrico G
			float NH2 = 2.0 * NdotH;
			float g1 = (NH2 * NdotV) / VdotH;
			float g2 = (NH2 * lambertian) / VdotH;
			float geoAtt = min(1.0, min(g1, g2));

			//Rugosità D
            //Distribuzione di Beckmann
			float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
			float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
			float roughness = r1 * exp(r2);

			//Riflettanza di Fresnel F con approssimazione di Schlick
			float fresnel = pow(1.0 - VdotH, 5.0);
			fresnel *= (1.0 - F0);
			fresnel += F0;

			//Calcolo la componente speculare
			specular = (fresnel * geoAtt * roughness) / (PI * NdotV * lambertian);
			
			//Calcolo colore finale per la directional light i e lo sommo al colore finale
			color += surfaceColor * lambertian * (Kd + specular * (1.0 - Kd));
		}
	}   

    colorFrag = color;
}
