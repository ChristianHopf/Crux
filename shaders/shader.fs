#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

//uniform vec3 lightPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main(){
  float ambientStrength = 0.1;
  float specularStrength = texture(specularMap, TexCoord).r;

  // Norm light dir
  vec3 norm = normalize(Normal);
  //vec3 lightDir = normalize(lightPos - norm);
  vec3 lightDir = normalize(-lightDirection);

  // Ambient
  vec3 ambient = ambientStrength * lightColor;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * texture(diffuseMap, TexCoord).rgb;

  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColor;

  vec3 result = ambient + diffuse + specular;
  //FragColor = vec4(result, 1.0);

  FragColor = vec4(diffuse, 1.0);
}
