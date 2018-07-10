#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;

// the light incidence direction of the directional light (passed as uniform)
uniform vec3 lightVector;

out vec4 V_eye;
out vec4 L_eye;
out vec4 N_eye;

void main(void){
	vec4 mvPosition = viewMatrix * modelMatrix * vec4( position, 1.0 );
	
	V_eye = -mvPosition.xyzw;
	
	L_eye = viewMatrix  * vec4(-lightVector, 0.0);
	
	vec4 N_eye = vec4( normalize( normalMatrix * normal ), 1.0);

	gl_Position = projectionMatrix * mvPosition;
}