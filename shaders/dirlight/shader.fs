#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 viewPos;

struct DirLight {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
uniform DirLight dirLight;

struct Material {
  sampler2D diffuse1;
  sampler2D diffuse2;
  sampler2D diffuse3;

  sampler2D specular1;
  sampler2D specular2;

  sampler2D normal;
};
uniform Material material;

vec3 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir);

void main(){
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // Directional light
  vec3 result = calc_dir_light(dirLight, norm, viewDir);

  FragColor = vec4(result, 1.0);
}

vec3 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir){
  // Light direction
  vec3 lightDir = normalize(-light.direction);

  // Ambient
  vec3 ambient = light.ambient * vec3(texture(material.diffuse1, TexCoord));

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse1, TexCoord));

  // Specular
  //vec3 reflectDir = reflect(-lightDir, norm);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
  vec3 specular = light.specular * spec * vec3(texture(material.specular1, TexCoord));

  return (ambient + diffuse + specular);
}
