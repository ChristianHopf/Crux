#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

void main(){
  vec3 diffuseColor = texture(diffuseMap, TexCoord).rgb;
  float specStrength = texture(specularMap, TexCoord).r;

  vec3 color = diffuseColor + vec3(specStrength);
  FragColor = vec4(color, 1.0);
}
