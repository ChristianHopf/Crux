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

  sampler2D emissive;

  sampler2D normal;
};
uniform Material material;

vec4 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir);

void main(){
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // Directional light
  vec4 result = calc_dir_light(dirLight, norm, viewDir);

  // Emissive light
  //vec4 emissive_vec4 = vec4(texture(material.emissive, TexCoord).rgb, 1.0);
  //vec3 emissive = emissive_vec4.rgb * emissive_vec4.a;
  vec3 emissive = texture(material.emissive, TexCoord).rgb;

  FragColor = vec4(result.rgb + emissive, result.a);
}

vec4 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir){
  // Light direction
  vec3 lightDir = normalize(-light.direction);

  // Ambient
  vec3 ambient = light.ambient * vec3(texture(material.diffuse1, TexCoord));

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(material.diffuse1, TexCoord).rgb;
  float alpha = texture(material.diffuse1, TexCoord).a;
  if (alpha < 0.1)
    discard;
  //vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse1, TexCoord));

  // Specular
  //vec3 reflectDir = reflect(-lightDir, norm);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
  vec3 specular = light.specular * spec * vec3(texture(material.specular1, TexCoord));

  return vec4(ambient + diffuse + specular, alpha);
}
