#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

uniform vec3 viewPos;

struct DirLight {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
uniform DirLight dirLight;

void calc_dir_light(DirLight light, vec3 norm, vec3 viewDir);

void main(){
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // Directional light
  vec3 result = calc_dir_light(dirLight, norm, viewDir);

  FragColor = vec4(result, 1.0);
}

vec3 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir){
  // Directional lightDir
  vec3 lightDir = normalize(-light.direction);
  // Ambient
  vec3 ambient = light.ambient * vec3(texture(diffuseMap, TexCoord));
  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseMap, TexCoord));
  // Specular
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = light.specular * spec * vec3(texture(specularMap, TexCoord));
  return (ambient + diffuse + specular);
}
