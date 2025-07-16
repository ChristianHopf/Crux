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
  vec3 emissive_color;
  bool has_emissive;

  float opacity;

  bool mask;
  float alphaCutoff;

  bool unlit;

  sampler2D normal;
};
uniform Material material;

vec4 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir);

void main(){
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  vec4 baseColor = texture(material.diffuse1, TexCoord);

  float alpha = baseColor.a;
  if (material.mask)
    if (alpha < material.alphaCutoff)
      discard;

  // Emissive light
  vec3 emissive = vec3(0.0f);
  if (material.has_emissive){
    //emissive = texture(material.emissive, TexCoord).rgb;
    emissive = texture(material.emissive, TexCoord).rgb * material.emissive_color;
  }
  //emissive = vec3(texture(material.emissive, TexCoord));

  // Directional light
  vec3 resultColor;
  if (!material.unlit){
    vec4 lighting = calc_dir_light(dirLight, norm, viewDir);
    resultColor = lighting.rgb + emissive;
  } else {
    resultColor = baseColor.rgb + emissive;
  }

  FragColor = vec4(resultColor, alpha);
  //FragColor = vec4(resultColor, alpha);
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

  // Specular
  //vec3 reflectDir = reflect(-lightDir, norm);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
  vec3 specular = light.specular * spec * vec3(texture(material.specular1, TexCoord));

  return vec4(ambient + diffuse + specular, alpha);
}
