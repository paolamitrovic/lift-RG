#version 330 core

struct Light{ //Svjetlosni izvor
	vec3 pos; //Pozicija
	vec3 kA; //Ambijentalna komponenta (Indirektno svjetlo)
	vec3 kD; //Difuzna komponenta (Direktno svjetlo)
	vec3 kS; //Spekularna komponenta (Odsjaj)
	float constant; //Konstantno slabljenje
	float linear; //Linearno slabljenje
	float quadratic; //Kvadratno slabljenje
};
struct Material{ //Materijal objekta
	vec3 kA;
	vec3 kD;
	vec3 kS;
	float shine; //Uglancanost
};

in vec4 channelCol;
in vec2 channelTex;
in vec3 chNor;
in vec3 chFragPos;

out vec4 outCol;

uniform sampler2D uTex;
uniform bool useTex;
uniform bool transparent;

uniform Light uLight; //Svjetlosni izvor (prvi - lampa na spratu)
uniform Light uLight2; //Drugi svjetlosni izvor (lampa u liftu)
uniform Material uMaterial; //Materijal objekta
uniform vec3 uViewPos; //Pozicija kamere (za racun spekularne komponente)

void main()
{
	vec3 normal = normalize(chNor);
	vec3 viewDirection = normalize(uViewPos - chFragPos);
	
	// Osnovna ambijentalna svetlost (ne zavisi od lampi) - smanjena jer imamo attenuation
	vec3 baseAmbient = vec3(0.15, 0.15, 0.15) * uMaterial.kA;
	
	// Prvi izvor svetlosti (lampa na spratu)
	vec3 lightDirection1 = uLight.pos - chFragPos;
	float distance1 = length(lightDirection1);
	lightDirection1 = normalize(lightDirection1);
	
	// Attenuation (slabljenje sa udaljenošću)
	float attenuation1 = 1.0 / (uLight.constant + uLight.linear * distance1 + uLight.quadratic * (distance1 * distance1));
	
	vec3 resA1 = uLight.kA * uMaterial.kA;
	float nD1 = max(dot(normal, lightDirection1), 0.0);
	vec3 resD1 = uLight.kD * (nD1 * uMaterial.kD) * attenuation1;
	vec3 reflectionDirection1 = reflect(-lightDirection1, normal);
	float s1 = pow(max(dot(viewDirection, reflectionDirection1), 0.0), uMaterial.shine);
	vec3 resS1 = uLight.kS * (s1 * uMaterial.kS) * attenuation1;
	
	// Drugi izvor svetlosti (lampa u liftu)
	vec3 lightDirection2 = uLight2.pos - chFragPos;
	float distance2 = length(lightDirection2);
	lightDirection2 = normalize(lightDirection2);
	
	// Attenuation (slabljenje sa udaljenošću)
	float attenuation2 = 1.0 / (uLight2.constant + uLight2.linear * distance2 + uLight2.quadratic * (distance2 * distance2));
	
	vec3 resA2 = uLight2.kA * uMaterial.kA;
	float nD2 = max(dot(normal, lightDirection2), 0.0);
	vec3 resD2 = uLight2.kD * (nD2 * uMaterial.kD) * attenuation2;
	vec3 reflectionDirection2 = reflect(-lightDirection2, normal);
	float s2 = pow(max(dot(viewDirection, reflectionDirection2), 0.0), uMaterial.shine);
	vec3 resS2 = uLight2.kS * (s2 * uMaterial.kS) * attenuation2;
	
	// Kombinuj oba izvora svetlosti
	// Ambijentalna komponenta - koristi maksimum (indirektno svetlo je globalno) + osnovna ambijentalna
	vec3 resA = baseAmbient + max(resA1, resA2);
	// Difuzna i spekularna komponenta - saberi (direktno svetlo se sabira)
	vec3 lighting = resA + resD1 + resS1 + resD2 + resS2;
	
	// Ograniči osvetljenje da ne bude previše svetlo
	lighting = min(lighting, vec3(2.0)); // Maksimalno osvetljenje
	
	if (!useTex) {
		outCol = vec4(channelCol.rgb * lighting, channelCol.a);
	}
	else {
		vec4 texColor = texture(uTex, channelTex);
		if (!transparent && texColor.a < 1) {
			texColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
		outCol = vec4(texColor.rgb * lighting, texColor.a);
	}
}