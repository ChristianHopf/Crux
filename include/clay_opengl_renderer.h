#pragma once

#include <glad/glad.h>
#include "clay.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cglm/cglm.h>
#include "shader.h"
#include "text.h"

// Forward declarations
// typedef struct Clay_RenderCommandArray Clay_RenderCommandArray;
// typedef struct Clay_RenderCommand Clay_RenderCommand;
// typedef struct Clay_BoundingBox Clay_BoundingBox;
// typedef struct Clay_RectangleRenderData Clay_RectangleRenderData;
// typedef struct Clay_Color Clay_Color;

struct ClayOpenGLRenderer {
  Shader *rectangle_shader;
  Shader *glyph_shader;

  GLuint text_VAO;
  GLuint text_VBO;
};


// Setup, teardown
void clay_opengl_renderer_init();
void clay_opengl_renderer_destroy();
bool clay_opengl_renderer_create_shaders();
bool clay_opengl_renderer_text_setup();

// Render
void clay_opengl_render(Clay_RenderCommandArray render_commands, struct Font *fonts);

// Primitive renderer functions
void clay_opengl_draw_rectangle(float x, float y, float width, float height, Clay_Color color);
void clay_opengl_draw_text(struct Font font, Clay_StringSlice text, float x, float y, float size, float spacing, float scale, Clay_Color color);

// Helpers
float *clay_color_to_vec4(Clay_Color clay_color);
