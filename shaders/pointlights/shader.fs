#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

uniform vec3 viewPos;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NUM_POINT_LIGHTS 4  
uniform PointLight pointLights[NUM_POINT_LIGHTS];

vec3 calc_point_light(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir);

void main(){
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // Point lights
  vec3 result = vec3(0.0);
  for(int i = 0; i < NUM_POINT_LIGHTS; i++){
    result += calc_point_light(pointLights[i], norm, FragPos, viewDir);
  }

  FragColor = vec4(result, 1.0);
}

vec3 calc_point_light(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
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
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = attenuation * light.specular * spec * vec3(texture(specularMap, TexCoord));

  return (ambient + diffuse + specular);
}
