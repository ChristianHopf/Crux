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
	Shader *shader = shader_create("shaders/shader.vs", "shaders/pointlights/shader.fs");
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
  model_load(crystalModel, "resources/objects/dragon_slayer_berserk.glb");
  Entity crystal = {
    .ID = 1,
    .position = {0.0f, 0.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {0.2f, 0.2f, 0.2f},
    .model = crystalModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = crystal;

  Shader *stencilShader = shader_create("shaders/shader.vs", "shaders/stencil/shader.fs");

  Entity scaledCrystal = {
    .ID = 1,
    .position = {0.0f, 0.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {0.21f, 0.21f, 0.21f},
    .model = crystalModel,
    .shader = stencilShader
  };
  //scene->entities[scene->num_entities++] = scaledCrystal;

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

void scene_update(Scene *scene, float deltaTime){
  printf("Scene update!\n");
  // Rotate around y axis
  float rotationSpeed = 100.0f;
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

    // Set lightPos and lightColor uniforms in the fragment shader
    //shader_set_vec3(entity->shader, "lightPos", (vec3){(float)(sin(glfwGetTime())*5), 0.5f, (float)(cos(glfwGetTime())*5)});
    //shader_set_vec3(entity->shader, "lightPos", (vec3){1.2f, 0.5f, 2.0f});
    
    // Point light uniforms
    // 1
    shader_set_vec3(entity->shader, "pointLights[0].position", (vec3){-2.0f, 2.0f, -2.0f});
    shader_set_vec3(entity->shader, "pointLights[0].ambient", (vec3){0.05f, 0.05f, 0.05f});
    shader_set_vec3(entity->shader, "pointLights[0].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    shader_set_vec3(entity->shader, "pointLights[0].specular", (vec3){1.0f, 1.0f, 1.0f});
    shader_set_float(entity->shader, "pointLights[0].constant", 1.0f);
    shader_set_float(entity->shader, "pointLights[0].linear", 0.14f);
    shader_set_float(entity->shader, "pointLights[0].quadratic", 0.07f);
    // 2
    shader_set_vec3(entity->shader, "pointLights[1].position", (vec3){2.0f, 2.0f, -2.0f});
    shader_set_vec3(entity->shader, "pointLights[1].ambient", (vec3){0.05f, 0.05f, 0.05f});
    shader_set_vec3(entity->shader, "pointLights[1].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    shader_set_vec3(entity->shader, "pointLights[1].specular", (vec3){1.0f, 1.0f, 1.0f});
    shader_set_float(entity->shader, "pointLights[1].constant", 1.0f);
    shader_set_float(entity->shader, "pointLights[1].linear", 0.14f);
    shader_set_float(entity->shader, "pointLights[1].quadratic", 0.07f);
    // 3
    shader_set_vec3(entity->shader, "pointLights[2].position", (vec3){2.0f, 2.0f, 2.0f});
    shader_set_vec3(entity->shader, "pointLights[2].ambient", (vec3){0.05f, 0.05f, 0.05f});
    shader_set_vec3(entity->shader, "pointLights[2].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    shader_set_vec3(entity->shader, "pointLights[2].specular", (vec3){1.0f, 1.0f, 1.0f});
    shader_set_float(entity->shader, "pointLights[2].constant", 1.0f);
    shader_set_float(entity->shader, "pointLights[2].linear", 0.14f);
    shader_set_float(entity->shader, "pointLights[2].quadratic", 0.07f);
    // 4
    shader_set_vec3(entity->shader, "pointLights[3].position", (vec3){-2.0f, 2.0f, 2.0f});
    shader_set_vec3(entity->shader, "pointLights[3].ambient", (vec3){0.05f, 0.05f, 0.05f});
    shader_set_vec3(entity->shader, "pointLights[3].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    shader_set_vec3(entity->shader, "pointLights[3].specular", (vec3){1.0f, 1.0f, 1.0f});
    shader_set_float(entity->shader, "pointLights[3].constant", 1.0f);
    shader_set_float(entity->shader, "pointLights[3].linear", 0.14f);
    shader_set_float(entity->shader, "pointLights[3].quadratic", 0.07f);

    // Set camera position as viewPos in the fragment shader
    shader_set_vec3(entity->shader, "viewPos", scene->camera->position);

    // Draw model
    model_draw(entity->model, entity->shader);
  }
}
