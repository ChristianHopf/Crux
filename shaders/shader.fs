#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

uniform vec3 viewPos;

struct Light {
  vec3 position;
  vec3 color;

  float constant;
  float linear;
  float quadratic;
};

uniform Light light;

void main(){
  // Attenuation
  float distance = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  float ambientStrength = 0.1;
  //float specularStrength = texture(specularMap, TexCoord).r;
  float specularStrength = 0.2;

  // Norm light dir
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - norm);

  // Ambient
  vec3 ambient = attenuation * ambientStrength * light.color;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = attenuation * diff * texture(diffuseMap, TexCoord).rgb;

  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = attenuation * specularStrength * spec * light.color;

  vec3 result = ambient + diffuse + specular;
  FragColor = vec4(result, 1.0);
}
