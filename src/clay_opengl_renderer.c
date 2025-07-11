#include "clay.h"
#include "clay_opengl_renderer.h"

static struct ClayOpenGLRenderer clay_opengl_renderer;

void clay_opengl_renderer_init(float screen_width, float screen_height){
  // The renderer maintains variables for screen width and height updated each frame
  clay_opengl_renderer.screen_width = screen_width;
  clay_opengl_renderer.screen_height = screen_height;

  // Create shaders
  if (!clay_opengl_renderer_create_shaders()){
    fprintf(stderr, "Error: failed to create shaders in clay_opengl_renderer_init\n");
    return;
  }

  // Load fonts
  if (!clay_opengl_renderer_text_setup()){
    fprintf(stderr, "Error: failed to load fonts in clay_opengl_renderer_init\n");
    return;
  }
}

void clay_opengl_renderer_destroy(){

}

bool clay_opengl_renderer_create_shaders(){
  // Create rectangle shader
  clay_opengl_renderer.rectangle_shader = shader_create("shaders/clay/rectangle/shader.vs", "shaders/clay/rectangle/shader.fs");
  if (!clay_opengl_renderer.rectangle_shader){
    fprintf(stderr, "Error: failed to create clay rectangle shader in ui_manager_init\n");
    return false;
  }

  // Create glyph shader
  clay_opengl_renderer.glyph_shader = shader_create("shaders/glyph/shader.vs", "shaders/glyph/shader.fs");
  if (!clay_opengl_renderer.glyph_shader){
    printf("Error: failed to create glyph shader in load_font_face\n");
    return false;
  }

  return true;
}

bool clay_opengl_renderer_text_setup(){
  // Allocate fonts
  // clay_opengl_renderer.fonts = (struct Font *)malloc(sizeof(struct Font));
  // if (!clay_opengl_renderer.fonts){
  //   fprintf(stderr, "Error: failed to allocate fonts in clay_opengl_renderer_init\n");
  //   return false;
  // }
  //
  // // Load fonts
  // clay_opengl_renderer.fonts = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  // if (!clay_opengl_renderer.fonts){
  //   fprintf(stderr, "Error: failed to load font in clay_opengl_renderer_init\n");
  //   return false;
  // }

  // Text VAO and VBO
  glGenVertexArrays(1, &clay_opengl_renderer.text_VAO);
  glGenBuffers(1, &clay_opengl_renderer.text_VBO);
  glBindVertexArray(clay_opengl_renderer.text_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, clay_opengl_renderer.text_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Should probably put some glGetError calls here

  return true;
}

void clay_opengl_renderer_update_dimensions(float screen_width, float screen_height){
  clay_opengl_renderer.screen_width = screen_width;
  clay_opengl_renderer.screen_height = screen_height;
}

void clay_opengl_render(Clay_RenderCommandArray renderCommands, struct Font *fonts){
  // Get render command
  for(int i = 0; i < renderCommands.length; i++){
    Clay_RenderCommand *render_command = Clay_RenderCommandArray_Get(&renderCommands, i);
    Clay_BoundingBox box = {roundf(render_command->boundingBox.x), roundf(render_command->boundingBox.y), roundf(render_command->boundingBox.width), roundf(render_command->boundingBox.height)};

    // Check type
    switch(render_command->commandType){
      case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
        Clay_RectangleRenderData *config = &render_command->renderData.rectangle;

        // No radius support yet
        clay_opengl_draw_rectangle(box.x, clay_opengl_renderer.screen_height - box.y, box.width, box.height, config->backgroundColor);
        break;
      }
      case CLAY_RENDER_COMMAND_TYPE_TEXT: {
        Clay_TextRenderData *config = &render_command->renderData.text;
        struct Font font_to_use = fonts[config->fontId];

        clay_opengl_draw_text(font_to_use, config->stringContents, box.x, clay_opengl_renderer.screen_height - (box.y + config->fontSize), config->fontSize, config->letterSpacing, 1.0f, config->textColor);
        break;
      }
    }
  }
}

void clay_opengl_draw_rectangle(float x, float y, float width, float height, Clay_Color color){
  // Use rectangle shader, set projection uniform based on window and color uniform
  shader_use(clay_opengl_renderer.rectangle_shader);
  mat4 orthographic;
  glm_ortho(0, clay_opengl_renderer.screen_width, 0, clay_opengl_renderer.screen_height, -1.0f, 1.0f, orthographic);
  shader_set_mat4(clay_opengl_renderer.rectangle_shader, "projection", orthographic);
  shader_set_vec4(clay_opengl_renderer.rectangle_shader, "color", (vec4) {color.r, color.g, color.b, color.a});

  // Create vertices
  float vertices[12] = {
    x,          y,          0.0f,
    x,          y - height, 0.0f,
    x + width,  y - height, 0.0f,
    x + width,  y,          0.0f,
  };
  // Create indices
  unsigned int indices[6] = {
    0, 1, 3,
    1, 2, 3
    // 0, 1, 2,
    // 0, 2, 3
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

void clay_opengl_draw_text(struct Font font, Clay_StringSlice text, float x, float y, float size, float spacing, float scale, Clay_Color color){
  // Use shader, set uniform, bind to texture and VAO
  shader_use(clay_opengl_renderer.glyph_shader);
  mat4 orthographic;
  glm_ortho(0, clay_opengl_renderer.screen_width, 0, clay_opengl_renderer.screen_height, -1.0f, 1.0f, orthographic);
  // glm_ortho(0, 1251, 0, 676, -1.0f, 1.0f, orthographic);
  shader_set_mat4(clay_opengl_renderer.glyph_shader, "projection", orthographic);
  shader_set_vec3(clay_opengl_renderer.glyph_shader, "textColor", (vec4) {color.r, color.g, color.b, color.a});

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(clay_opengl_renderer.text_VAO);

  // Draw each character in text
  glDisable(GL_DEPTH_TEST);
  for(int i = 0; i < text.length; i++){
    // Get its Character struct, x, y, w, and h
    struct Character text_char = font.characters[(int)text.chars[i]];

    float xpos = x + text_char.bearing[0] * scale;
    float ypos = y - (text_char.size[1] - text_char.bearing[1]) * scale;

    float w = text_char.size[0] * scale;
    float h = text_char.size[1] * scale;

    // New vertices to buffer to VBO
    float vertices[6][4] = {
      { xpos,     ypos + h,   0.0f, 0.0f },            
      { xpos,     ypos,       0.0f, 1.0f },
      { xpos + w, ypos,       1.0f, 1.0f },

      { xpos,     ypos + h,   0.0f, 0.0f },
      { xpos + w, ypos,       1.0f, 1.0f },
      { xpos + w, ypos + h,   1.0f, 0.0f }           
    };

    // Render glyph texture over character quad
    glBindTexture(GL_TEXTURE_2D, text_char.texture_id);
    glBindBuffer(GL_ARRAY_BUFFER, clay_opengl_renderer.text_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Move cursor to the next character's position
    x += (text_char.advance >> 6) * scale;
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glEnable(GL_DEPTH_TEST);
}

float *clay_color_to_vec4(Clay_Color clay_color){
  vec4 color;
  color[0] = clay_color.r;
  color[1] = clay_color.g;
  color[2] = clay_color.b;
  color[3] = clay_color.a;

  return color;
}
