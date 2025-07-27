#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <cglm/cglm.h>
#include "shader.h"

struct Character {
  GLuint texture_id;
  vec2 size;
  vec2 bearing;
  unsigned int advance;
};

struct Font {
  // unsigned int font_id;
  struct Character characters[128];
  int base_size;
};


struct Font *load_font_face(char *path, int size);
void text_render(char *text, float x, float y, float scale, vec3 color);
