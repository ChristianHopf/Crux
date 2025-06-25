#version 330 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform Matrices{
  mat4 view;
  mat4 projection;
};

out vec3 TexCoords;

void main(){
  mat4 skybox_view = mat4(mat3(view));
  TexCoords = aPos;
  vec4 pos = projection * skybox_view * vec4(aPos, 1.0);
  gl_Position = pos.xyww;
}
