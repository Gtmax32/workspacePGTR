#version 330 core

const float PI = 3.14159;

// output shader variable
out vec4 colorFrag;

// light incidence direction (calculated in vertex shader, interpolated by rasterization)
in vec3 lightDir;
// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 vNormal;
// vector from fragment to camera (in view coordinate)
in vec3 vViewPosition;

uniform vec3 diffuseColor; 
uniform float m; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence
uniform float Kd; // fraction of diffuse reflection (specular reflection = 1 - k)

void main(){
	// normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir.xyz);

    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);

    float specular = 0.0;

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0){
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( vViewPosition );

        // half vector
        vec3 H = normalize(L + V);
       
        // implementazione delle formule viste nelle slide
        // spezzo in componenti

        // we calculate the cosine and parameters to be used in the different components
        float NdotH = max(dot(N, H), 0.0); 
        float NdotV = max(dot(N, V), 0.0); 
        float VdotH = max(dot(V, H), 0.0);
        float mSquared = m * m;
        
        // Geometric factor G
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * lambertian) / VdotH;
        float geoAtt = min(1.0, min(g1, g2));

        // Rugosity D
        // Beckmann Distribution
        // we can simplify the tangent at the exponent in this way:
        // tan = sen/cos -> tan^2 = sen^2/cos^2 -> tan^2 = (1-cos^2)/cos^2
        // thus, the exponent becomes -(1-cos^2)/(m^2*cos^2) -> (cos^2-1)/(m^2*cos^2)
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float roughness = r1 * exp(r2);
        
        // Fresnel reflectance F (approx Schlick)
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;
        
        // we put everything together for the specular component
        specular = (fresnel * geoAtt * roughness) / (NdotV * lambertian * PI);
    }
    
    // calcolo colore finale con anche la componente diffusiva
    vec3 finalColor = diffuseColor * lambertian * (Kd + specular * (1.0 - Kd));
    colorFrag = vec4(finalColor, 1.0);
}