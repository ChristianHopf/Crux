#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform vec3 lightPos;
unifrom vec3 lightColor;

void main(){
  //vec3 diffuseColor = texture(diffuseMap, TexCoord).rgb;
  //float specStrength = texture(specularMap, TexCoord).r;
  float ambientStrength = 0.1;

  // Norm light dir
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - norm);

  // Ambient
  vec3 ambient = ambientStrength * lightColor;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  //vec3 diffuse = diff * lightColor;
  vec3 diffuse = diff * texture(diffuseMap, TexCoord).rgb;

  vec3 result = ambient + diffuse;
  FragColor = vec4(result, 1.0);
  //vec3 color = diffuseColor + vec3(specStrength);
  //FragColor = vec4(color, 1.0);
}
