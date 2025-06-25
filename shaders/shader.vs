#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform Matrices{
  mat4 view;
  mat4 projection;
};

out vec3 Normal;
out vec2 TexCoord;
out vec3 FragPos;

uniform mat3 normal;
uniform mat4 model;

void main(){
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  Normal = normalize(normal * aNormal);
  //Normal = aNormal;
  TexCoord = aTexCoord;
  FragPos = vec3(model * vec4(aPos, 1.0));
}
