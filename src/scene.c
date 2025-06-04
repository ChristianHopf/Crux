#include "scene.h"
#include <GL/gl.h>
#include <iso646.h>
#include <math.h>

Scene *scene_create(){
  // Allocate scene
  Scene *scene = (Scene *)malloc(sizeof(Scene));
  if (!scene){
    printf("Error: failed to allocate scene\n");
    return NULL;
  }

  // Light
  scene->light = (Light *)malloc(sizeof(Light));
  if (!scene->light){
    printf("Error: failed to allocate scene light\n");
    return NULL;
  }
  glm_vec3_copy({-0.2f, -1.0f, -0.3f}, scene->light->direction);
  glm_vec3_copy({0.2f, 0.2f, 0.2f}, scene->light->ambient);
  glm_vec3_copy({0.8f, 0.8f, 0.8f}, scene->light->diffuse);
  glm_vec3_copy({1.0f, 1.0f, 1.0f}, scene->light->specular);

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
	Shader *shader = shader_create("shaders/shader.vs", "shaders/dirlight/shader.fs");
	if (!shader->ID){
		printf("Error: failed to create shader program\n");
		glfwTerminate();
		return NULL;
	}

  Model *crystalModel = (Model *)malloc(sizeof(Model));
  if (!crystalModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  model_load(crystalModel, "resources/objects/crystal/scene.gltf");
  Entity crystal = {
    .ID = 1,
    .position = {0.0f, 0.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {0.2f, 0.2f, 0.2f},
    .model = crystalModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = crystal;

  Model *oiiaiModel = (Model *)malloc(sizeof(Model));
  if (!oiiaiModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  //model_load(oiiaiModel, "resources/objects/oiiai/scene.gltf");
  Entity oiiai = {
    .ID = 1,
    .position = {0.0f, 0.0f, -10.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {0.1f, 0.1f, 0.1f},
    .model = oiiaiModel,
    .shader = shader
  };
  //scene->entities[scene->num_entities++] = oiiai;

  return scene;
}

void scene_update(Scene *scene, float deltaTime){
  printf("Scene update!\n");
  // Rotate around y axis
  float rotationSpeed = 100.0f;

  // Update entities
  for(int i = 0; i < scene->num_entities; i++){
    Entity *entity = &scene->entities[i];

    // Update translation vector
    entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
    // Update rotation vector
    entity->rotation[1] += rotationSpeed * deltaTime;
  }
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

printf("DirLight: ambient=(%f,%f,%f), diffuse=(%f,%f,%f), specular=(%f,%f,%f)\n",
       scene->light->ambient[0], scene->light->ambient[1], scene->light->ambient[2],
       scene->light->diffuse[0], scene->light->diffuse[1], scene->light->diffuse[2],
       scene->light->specular[0], scene->light->specular[1], scene->light->specular[2]);

    // Lighting uniforms
    shader_set_vec3(entity->shader, "dirLight.direction", scene->light->direction);
    shader_set_vec3(entity->shader, "dirLight.ambient", scene->light->ambient);
    shader_set_vec3(entity->shader, "dirLight.diffuse", scene->light->diffuse);
    shader_set_vec3(entity->shader, "dirLight.specular", scene->light->specular);

    // Set camera position as viewPos in the fragment shader
    shader_set_vec3(entity->shader, "viewPos", scene->camera->position);

    // Draw model
    model_draw(entity->model, entity->shader);
  }
}
