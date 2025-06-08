#version 330 core

in VS_OUT {
  vec2 TexCoord;
  //vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
} fs_in;

out vec4 FragColor;

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

  // Sample normal map
  vec3 normal = texture(material.normal, fs_in.TexCoord).rgb;
  normal = normalize(normal * 2.0 - 1.0);

  // View direction
  vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

  // Directional light
  vec4 lighting = calc_dir_light(dirLight, normal, viewDir);

  // Emissive light
  vec4 emissive = texture(material.emissive, fs_in.TexCoord);

  vec3 resultColor = lighting.rgb + emissive.rgb;
  float resultAlpha = lighting.a;

  FragColor = vec4(lighting.rgb + emissive.rgb, resultAlpha);
}

vec4 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir){
  // Light direction
  vec3 lightDir = normalize(-light.direction);

  // Ambient
  vec3 ambient = light.ambient * vec3(texture(material.diffuse1, fs_in.TexCoord));

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(material.diffuse1, fs_in.TexCoord).rgb;
  float alpha = texture(material.diffuse1, fs_in.TexCoord).a;
  //if (alpha < 0.1)
  //  discard;

  // Specular
  //vec3 reflectDir = reflect(-lightDir, norm);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
  vec3 specular = light.specular * spec * vec3(texture(material.specular1, fs_in.TexCoord));

  return vec4(ambient + diffuse + specular, alpha);
}
