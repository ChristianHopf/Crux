#include "clay.h"
#include "clay_opengl_renderer.h"

static struct ClayOpenGLRenderer clay_opengl_renderer;

void clay_opengl_renderer_init(){
  if (!clay_opengl_renderer_create_shaders()){
    fprintf(stderr, "Error: failed to create shaders in clay_opengl_renderer_init\n");
  }
}

bool clay_opengl_renderer_create_shaders(){
  // Create rectangle shader
  clay_opengl_renderer.rectangle_shader = shader_create("shaders/clay/rectangle/shader.vs", "shaders/clay/rectangle/shader.fs");
  if (!clay_opengl_renderer.rectangle_shader){
    fprintf(stderr, "Error: failed to create clay rectangle shader in ui_manager_init\n");
    return false;
  }
  return true;
}

void clay_opengl_render(Clay_RenderCommandArray renderCommands){
  // Get render command
  for(int i = 0; i < renderCommands.length; i++){
    Clay_RenderCommand *render_command = Clay_RenderCommandArray_Get(&renderCommands, i);
    Clay_BoundingBox box = {roundf(render_command->boundingBox.x), roundf(render_command->boundingBox.y), roundf(render_command->boundingBox.width), roundf(render_command->boundingBox.height)};

    // Check type
    switch(render_command->commandType){
      case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
        Clay_RectangleRenderData *config = &render_command->renderData.rectangle;

        // No radius support yet
        clay_opengl_draw_rectangle(box.x, box.y, box.width, box.height, config->backgroundColor);
      }
    }
  }
}

void clay_opengl_draw_rectangle(float x, float y, float width, float height, Clay_Color color){
  // Use rectangle shader, set projection uniform based on window and color uniform
  shader_use(clay_opengl_renderer.rectangle_shader);
  mat4 orthographic;
  glm_ortho(0, 1920, 0, 1080, -1.0f, 1.0f, orthographic);
  shader_set_mat4(clay_opengl_renderer.rectangle_shader, "projection", orthographic);
  shader_set_vec4(clay_opengl_renderer.rectangle_shader, "color", (vec4) {color.r, color.g, color.b, color.a});

  // Create vertices
  float vertices[12] = {
    x,          y,          0.0f,
    x + width,  y,          0.0f,
    x + width,  y + height, 0.0f,
    x,          y + height, 0.0f,
  };
  // Create indices
  unsigned int indices[6] = {
    0, 1, 2,
    0, 2, 3
  };

  // Buffer data
  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

  // Configure vertex attrib pointer
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

  // Draw
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glDisable(GL_DEPTH_TEST);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glEnable(GL_DEPTH_TEST);

  // Delete buffers, unbind and delete VAO
  // (worry about optimizing this later)
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteVertexArrays(1, &VAO);
}

float *clay_color_to_vec4(Clay_Color clay_color){
  vec4 color;
  color[0] = clay_color.r;
  color[1] = clay_color.g;
  color[2] = clay_color.b;
  color[3] = clay_color.a;

  return color;
}
