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

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
  vec3 position;
  vec3 direction;

  float cutoff;
  float outerCutoff;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
uniform SpotLight spotLight;

vec3 calc_dir_light(DirLight light, vec3 norm, vec3 viewDir);
vec3 calc_point_light(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir);
vec3 calc_spot_light(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir);

void main(){
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // Directional light
  vec3 result = CalcDirLight(dirLight, norm, viewDir);

  // Point light
  for(int i = 0; i < NR_POINT_LIGHTS; i++){
    result += calc_point_light(pointLights[i], norm, fragPos, viewDir);
  }

  // Spotlight
  result += calc_spot_light(spotLight, norm, fragPos, viewDir);

  FragColor = vec4(result, 1.0);
}

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 viewDir){
  // Directional lightDir
  vec3 lightDir = normalize(-light.direction);
  // Ambient
  vec3 ambient = light.ambient * vec3(texture(diffuseMap, TexCoord));
  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseMap, TexCoord));
  // Specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = light.specular * spec * vec3(texture(specularMap, TexCoord));
  return (ambient + diffuse + specular);
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
  vec3 lightDir = normalize(light.position - fragPos);
  // Attenuation
  float distance = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  // Ambient
  vec3 ambient = attenuation * light.ambient * vec3(texture(diffuseMap, TexCoord));

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = attenuation * light.diffuse * diff * vec3(texture(diffuseMap, TexCoord));

  // Specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = attenuation * light.specular * spec * vec3(texture(specularMap, TexCoord));

  return (ambient + diffuse + specular);
}

vec3 calc_spot_light(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
  vec3 lightDir = normalize(light.position - fragPos);

  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutoff - light.outerCutoff;
  float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = intensity * diff * texture(diffuseMap, TexCoord).rgb;

  // Specular
  vec3 viewDir = normalize(light.position - ragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = intensity * specularStrength * spec * light.color;

  return (diffuse + specular);
}
