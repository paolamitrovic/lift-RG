#version 330 core
out vec4 FragColor;

in vec3 chNormal;  
in vec3 chFragPos;  
in vec2 chUV;
  
uniform vec3 uLightPos; 
uniform vec3 uViewPos; 
uniform vec3 uLightColor;

uniform sampler2D uDiffMap1;
uniform bool uUseColor;  // Da li da koristi boju umesto teksture
uniform vec3 uModelColor; // Boja modela (ako se koristi umesto teksture)

void main()
{    
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * uLightColor;
  	
    // diffuse 
    vec3 norm = normalize(chNormal);
    vec3 lightDir = normalize(uLightPos - chFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // specular
    float specularStrength = 0.2;
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor;  

    // Koristi boju ili teksturu
    vec4 baseColor;
    if (uUseColor) {
        baseColor = vec4(uModelColor, 1.0);
    } else {
        vec4 texColor = texture(uDiffMap1, chUV);
        // Ako je tekstura bela (nije učitana), koristi boju
        if (texColor.r > 0.99 && texColor.g > 0.99 && texColor.b > 0.99) {
            baseColor = vec4(uModelColor, 1.0);
        } else {
            baseColor = texColor;
        }
    }
    
    FragColor = baseColor * vec4(ambient + diffuse + specular, 1.0);
}
