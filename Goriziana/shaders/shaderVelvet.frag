#version 330 core

uniform vec3 diffuseColor;

out vec4 colorFrag;

in vec4 V_eye;
in vec4 L_eye;
in vec4 N_eye;

const float backscatter = 0.1;
const float edginess = 20.5;
const float sheen = 0.81;
const float roughness = 0.1;

const vec4 Ka = vec4(0.23, 0.23, 0.23, 1.0);
const vec4 Kd = vec4(0.43, 0.43, 0.43, 1.0);

void main(void){
	vec3 V = normalize(vec3(V_eye));
	vec3 L = normalize(vec3(L_eye));
	vec3 N = normalize(vec3(N_eye));

	float diffuse = max(dot(L, N), 0.0);

	float cosine = max(dot(L, V), 0.0);
	float shiny = sheen * pow(cosine, 1.0 / roughness) * backscatter;

	cosine = max(dot(N, V), 0.0);
	float sine = sqrt(1.0 - cosine);
	shiny += sheen * pow(sine, edginess) * diffuse;

	colorFrag = Ka + (Kd * diffuse) + vec4(diffuseColor, 1.0) * shiny;
}