#include <cglm/cam.h>
#include <glad/glad.h>
// #include <iso646.h>
#include "text.h"

static struct Character characters[128];
static unsigned int VAO, VBO;
static Shader *textShader;


struct Font *load_font_face(char *path, int size){
  // Allocate Font
  // struct Font font = {0};
  struct Font *font = (struct Font *)malloc(sizeof(struct Font));
  if (!font){
    fprintf(stderr, "Error: failed to allocate font in load_font_face\n");
    return NULL;
  }

  // Init FreeType library, face
  FT_Library  library;
  FT_Face     face;

  int error = (FT_Init_FreeType(&library));
  if (error){
    fprintf(stderr, "Error: failed to initialize FreeType library\n");
    return NULL;
  }
  error = FT_New_Face(library, path, 0, &face);
  if (error == FT_Err_Unknown_File_Format){
    fprintf(stderr, "Error: failed to read font file: format unsupported\n");
    return NULL;
  }
  else if(error){
    fprintf(stderr, "Error: failed to read font file\n");
    return NULL;
  }

  // Set font size
  FT_Set_Pixel_Sizes(face, 0, size);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Process each character in the font face
  for (unsigned char c = 0; c < 128; c++){
    error = FT_Load_Char(face, c, FT_LOAD_RENDER);
    if (error){
      printf("ERROR::FREETYPE: Failed to load glyph for char %c\n", c);
      continue;
    }

    // Generate texture for each character
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Store in characters
    struct Character character = {
      .texture_id = texture_id,
      .size = {face->glyph->bitmap.width, face->glyph->bitmap.rows},
      .bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top},
      .advance = face->glyph->advance.x
    };
    font->characters[(int)c] = character;
  }
  font->base_size = size;

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  // Enable blending
  // glEnable(GL_CULL_FACE);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  return font;
}

void text_render(char *text, float x, float y, float scale, vec3 color){
  // Use shader, set uniform, bind to texture and VAO
  shader_use(textShader);
  shader_set_vec3(textShader, "textColor", color);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  // Draw each character in text
  glDisable(GL_DEPTH_TEST);
  for(size_t i = 0; i < strlen(text); i++){
    // Get its Character struct, x, y, w, and h
    struct Character text_char = characters[(int)text[i]];

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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
