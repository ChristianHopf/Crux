#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

//uniform vec3 lightDirection;

struct Light {
  vec3 position;
  vec3 color;

  float constant;
  float linear;
  float quadratic;
};

uniform Light light;

uniform vec3 viewPos;

void main(){
  // Attenuation
  float distance = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  float ambientStrength = 0.1;
  float specularStrength = texture(specularMap, TexCoord).r;

  // Norm light dir
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - norm);
  //vec3 lightDir = normalize(-lightDirection);

  // Ambient
  vec3 ambient = ambientStrength * light.color * attenuation;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * texture(diffuseMap, TexCoord).rgb * attenuation;

  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * light.color * attenuation;

  vec3 result = ambient + diffuse;
  //FragColor = vec4(result, 1.0);

  FragColor = vec4(result, 1.0);
}
