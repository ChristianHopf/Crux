#ifndef TEXT_H
#define TEXT_H

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


void load_font_face();
void text_render(char *text, float x, float y, float scale, vec3 color);

#endif
