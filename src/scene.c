#include "scene.h"
#include <math.h>

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
  scene->num_entities = 0;
  scene->max_entities = 5;
  scene->entities = (Entity *)malloc(scene->max_entities * sizeof(Entity));
  if (!scene->entities){
    printf("Error: failed to allocate scene entities\n");
    free(scene);
    return NULL;
  }

  // Model shader (for now, only use one shader)
	Shader *shader = shader_create("shaders/shader.vs", "shaders/shader.fs");
	if (!shader->ID){
		printf("Error: failed to create shader program\n");
		glfwTerminate();
		return NULL;
	}

  // Pochita
  Entity *my_entity = (Entity *)malloc(sizeof(Entity));
  if (!my_entity){
    printf("Error: failed to allocate entity\n");
    free(scene->entities);
    free(scene);
    return NULL;
  }
  my_entity->ID = 1;

  glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, my_entity->position);
  glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, my_entity->rotation);
  glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, my_entity->scale);
  //Model *model = model_create("resources/objects/pochita/scene.gltf");

  // Allocate and load model
  Model *model = (Model *)malloc(sizeof(Model));
  if (!model){
    printf("Error: failed to create Model\n");
    return NULL;
  }
  model_load(model, "resources/objects/backpack/backpack.obj");
  my_entity->model = model;
  my_entity->shader = shader;
  scene->entities[0] = *my_entity;
  scene->num_entities = 1;

  // Make a bunch of backpacks
  //for(int i = 0; i < 5; i++){
  //  Entity *backpack = (Entity *)malloc(sizeof(Entity));
  //  if (!backpack){
  //    printf("Error: failed to allocate backpack entity\n");
  //    free(scene->entities);
  //    free(scene);
  //    return NULL;
  //  }
  //  backpack->ID = i;
  //  glm_vec3_copy((vec3){(float)(5 * i), 0.0f, 0.0f}, backpack->position);
  //  glm_vec3_copy((vec3){(float)(45 * i), (float)(45 * i), (float)(45 * i)}, backpack->rotation);
  //  glm_vec3_copy((vec3){(float)(i + 1), (float)(i + 1), (float)(i + 1)}, backpack->scale);
  //  Model *model = model_create("resources/objects/backpack/backpack.obj");
  //  if (!model){
  //    printf("Error: failed to create Model\n");
  //  }
  //  backpack->model = model;
  //  backpack->shader = shader;
  //  scene->entities[i] = *backpack;
  //  scene->num_entities = i + 1;
  //}

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
  glm_perspective(glm_rad(scene->camera->fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

  // For each entity in the scene
  for(int i = 0; i < scene->num_entities; i++){
    // Bind its shader
    Entity *entity = &scene->entities[i];
    shader_use(entity->shader);

    // Get its model matrix
    mat4 model;
    glm_mat4_identity(model);
    // Apply transformations to model matrix
    // Translate
    glm_translate(model, entity->position);
    // Rotate
    glm_rotate_y(model, glm_rad(entity->rotation[1]), model);
    glm_rotate_x(model, glm_rad(entity->rotation[0]), model);
    glm_rotate_z(model, glm_rad(entity->rotation[2]), model);
    // Scale
    glm_scale(model, entity->scale);

    // Set its model, view, and projection matrix uniforms
    shader_set_mat4(entity->shader, "model", model);
    shader_set_mat4(entity->shader, "view", view);
    shader_set_mat4(entity->shader, "projection", projection);

    // Draw model
    model_draw(entity->model, entity->shader);
  }
}
