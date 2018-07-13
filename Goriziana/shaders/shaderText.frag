#version 330 core
in vec2 TexCoords;

out vec4 colorFrag;

//Immagine mono-colore bitmap del glyph
uniform sampler2D text;
//Colore da dare al testo
uniform vec3 textColor;

void main(){
	//Campiono il colore della texture come alpha value, in modo da gestire la trasparenza
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
	
	//Il valore di output del fragment è il colore effettivo da dare al testo
	colorFrag = vec4(textColor, 1.0) * sampled;
}