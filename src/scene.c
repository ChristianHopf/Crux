#include "scene.h"

Scene *scene_create(){
  // Allocate scene
  Scene *scene = (Scene *)malloc(sizeof(Scene));
  if (!scene){
    printf("Error: failed to allocate scene\n");
    return NULL;
  }

  // Camera
  scene->camera = camera_create((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f, 45.0f, 0.1f, 2.5f);
  if (!scene->camera){
    printf("Error: failed to create camera\n");
    free(scene->entities);
    free(scene);
    return NULL;
  }

  // Entities
  scene->num_entities = 1;
  scene->max_entities = 1;
  scene->entities = (Entity *)malloc(scene->max_entities * sizeof(Entity));
  if (!scene->entities){
    printf("Error: failed to allocate scene entities\n");
    free(scene);
    return NULL;
  }

  // Load our backpack model
  // Later make this use some kind of loading function
  Entity *backpack = (Entity *)malloc(sizeof(Entity));
  if (!backpack){
    printf("Error: failed to allocate backpack entity\n");
    free(scene->entities);
    free(scene);
    return NULL;
  }
  backpack->ID = 1;
  Model *model = model_create("resources/objects/backpack/backpack.obj");
  if (!model){
    printf("Error: failed to create Model\n");
  }
  backpack->model = model;

  // Model shader
	Shader *shader = shader_create("shaders/shader.vs", "shaders/shader.fs");
	if (!shader->ID){
		printf("Error: failed to create shader program\n");
		glfwTerminate();
		return NULL;
	}
  backpack->shader = shader;

  // Add backpack to entities
  scene->entities[0] = *backpack;

  return scene;
}

void scene_render(Scene *scene){
  // Render (clear color and depth buffer bits)
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get view and projection matrices
  mat4 view;
  mat4 projection;
  camera_get_view_matrix(scene->camera, view);
  glm_perspective(glm_rad(scene->camera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, projection);

  // For each entity in the scene
  for(int i = 0; i < scene->num_entities; i++){
    // Bind its shader
    Entity *entity = &scene->entities[i];
    shader_use(entity->shader);

    // Get its model matrix
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_scale(model, (vec3){1.0f, 1.0f, 1.0f});
    shader_set_mat4(entity->shader, "model", model);

    // Set its model, view, and projection matrix uniforms
    shader_set_mat4(entity->shader, "model", model);
    shader_set_mat4(entity->shader, "view", view);
    shader_set_mat4(entity->shader, "projection", projection);

    // Draw model
    model_draw(entity->model, entity->shader);
  }
}
