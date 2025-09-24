#version 330 core

in vec2 TexCoords;
//in vec3 textColor;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main(){
  // Transparent text background
  vec4 textSample = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);

  color = vec4(textColor, 1.0) * textSample;
}
