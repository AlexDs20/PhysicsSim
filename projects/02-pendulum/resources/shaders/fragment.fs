#version 330 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

// viewPos is here needed only because we compute the lighting in world space and not viewspace
// We can actually compute in view space and not need viewPos here because viewPos is always (0,0,0) in that case.
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
   // Ambient light
   float ambientStrength = 0.3;
   vec3 ambient = ambientStrength * lightColor;

   // Diffuse light
   vec3 norm = normalize(Normal);
   vec3 lightDir = normalize(lightPos - FragPos);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse = diff * lightColor;

   // Specular light
   float specularStrength = 0.5;
   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 reflectDir = reflect(-lightDir, norm);
   float shininess = 32;
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
   vec3 specular = specularStrength * spec * lightColor;

   vec3 result = (ambient + diffuse + specular) * objectColor;

   FragColor = vec4(result, 1.0);
}
