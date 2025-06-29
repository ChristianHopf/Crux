#include "menu.h"

void pause_menu_render(){
  // Render (clear color and depth buffer bits)
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  printf("rendering text\n");
  // Render text
  text_render("RESUME", 960.0f, 540.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
  text_render("EXIT", 960.0f, 500.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
}
