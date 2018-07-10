#version 330 core

struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// light component
uniform Light sunLight;

// object material component
uniform Material material;

// uniform that contain camera position, for Phong model
//uniform vec3 viewPos;

// output shader variable
out vec4 colorFrag;

in vec3 fragmentPos;

// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 vNormal;
// vector from fragment to camera (in view coordinate)
in vec3 vViewPosition;

// weight of the components
uniform float Ka;
uniform float Kd;
uniform float Ks;

// shininess coefficients (passed from the application)
uniform float shininess;

void main(){
	vec3 color;
	
    // ambient component
    vec3 ambient = Ka * sunLight.ambient * material.ambient;
	
	// normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    
	// normalization of the per-fragment light incidence direction
	vec3 L = normalize(sunLight.direction);
	
	// Lambert coefficient
	float lambertian = max(dot(L, N), 0.0);
	
	// if the lambert coefficient is positive, then I can calculate the specular component
	if(lambertian > 0.0) {
		vec3 diffuse = Kd * lambertian * material.diffuse * sunLight.diffuse;
		
		// the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
		vec3 V = normalize( vViewPosition );

		// in the Blinn-Phong model we do not use the reflection vector, but the half vector
		vec3 H = normalize(L + V);
		
		// in Phong model we use the reflection vector
		//vec3 viewDir = normalize(viewPos - fragmentPos);
		//vec3 reflectDir = reflect(-L, N);		
		//float specAngle = max(dot(viewDir, reflectDir), 0.0);
		
		// we use H to calculate the specular component
		float specAngle = max(dot(H, N), 0.0);
		// shininess application to the specular component
		float specularValue = pow(specAngle, shininess);
		
		vec3 specular = Ks * specularValue * material.specular * sunLight.specular;
		// We add diffusive and specular components to the final color
		color = ambient + diffuse + specular;
	}
    
	colorFrag  = vec4(color, 1.0);

}