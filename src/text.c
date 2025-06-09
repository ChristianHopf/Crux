#include <glad/glad.h>
#include "text.h"

void load_font_face(){
  // Init FreeType library, face
  FT_Library  library;
  FT_Face     face;

  int error = (FT_Init_FreeType(&library));
  if (error){
    printf("Error: failed to initialize FreeType library\n");
  }
  error = FT_New_Face(library, "resources/fonts/HackNerdFontMono-Regular.ttf", 0, &face);
  if (error == FT_Err_Unknown_File_Format){
    printf("Error: failed to read font file: format unsupported\n");
  }
  else if(error){
    printf("Error: failed to read font file\n");
  }

  // Set font size
  FT_Set_Pixel_Sizes(face, 0, 48);

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
    characters[(int)c] = character;
  }
  FT_Done_Face(face);
  FT_Done_FreeType(library);

  // VAO and VBO
}
