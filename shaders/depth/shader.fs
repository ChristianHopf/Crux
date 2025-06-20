#version 330 core

out vec4 FragColor;

float near = 0.1;
float far = 100.0;

void main(){
  float ndc = gl_FragCoord.z * 2.0 - 1.0;
  float linearDepth = ((2.0 * near * far) / (far + near - ndc * (far - near))) / far;

  FragColor = vec4(vec3(linearDepth), 1.0);
}
