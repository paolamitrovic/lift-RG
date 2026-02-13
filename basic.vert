#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec2 inTex;
layout(location = 3) in vec3 inNor; //Normale

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija

out vec4 channelCol;
out vec2 channelTex;
out vec3 chFragPos; //Interpolirana pozicija fragmenta
out vec3 chNor; //Interpolirane normale

void main()
{
	chFragPos = vec3(uM * vec4(inPos, 1.0));
	gl_Position = uP * uV * vec4(chFragPos, 1.0);
	channelCol = inCol;
	channelTex = inTex;
	// Transformiši normale - koristi samo rotacioni deo matrice (bez skaliranja)
	// Za jednostavnije transformacije (samo rotacija i translacija), možemo koristiti samo rotacioni deo
	mat3 normalMatrix = mat3(transpose(inverse(uM)));
	chNor = normalize(normalMatrix * inNor);
}