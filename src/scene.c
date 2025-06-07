#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/mat3.h>
#include "scene.h"
#include "player.h"
#include "utils.h"

Scene *scene_create(){
  // Allocate scene
  Scene *scene = (Scene *)malloc(sizeof(Scene));
  if (!scene){
    printf("Error: failed to allocate scene\n");
    return NULL;
  }

  // Start unpaused
  scene->paused = false;

  // Light
  scene->light = (Light *)malloc(sizeof(Light));
  if (!scene->light){
    printf("Error: failed to allocate scene light\n");
    return NULL;
  }
  glm_vec3_copy((vec3){-0.2f, 0.0f, -0.3f}, scene->light->direction);
  glm_vec3_copy((vec3){0.2f, 0.2f, 0.2f}, scene->light->ambient);
  glm_vec3_copy((vec3){0.8f, 0.8f, 0.8f}, scene->light->diffuse);
  glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, scene->light->specular);

  // Player
  player_init(&scene->player);

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
    .scale = {10.0f, 10.0f, 10.0f},
    .model = crystalModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = crystal;

  // Model *oiiaiModel = (Model *)malloc(sizeof(Model));
  // if (!oiiaiModel){
  //   printf("Error: failed to allocate oiiaiModel\n");
  //   return NULL;
  // }
  // model_load(oiiaiModel, "resources/objects/oiiai/scene.gltf");
  // Entity oiiai = {
  //   .ID = 1,
  //   .position = {0.0f, 0.0f, 0.0f},
  //   .rotation = {0.0f, 0.0f, 0.0f},
  //   .scale = {3.0f, 3.0f, 3.0f},
  //   .model = oiiaiModel,
  //   .shader = shader
  // };
  // scene->entities[scene->num_entities++] = oiiai;
  
  

  return scene;
}

void scene_update(Scene *scene, float deltaTime){
  // Skip update if the scene is paused
  if (scene->paused){
    return;
  }

  static float total_time = 0.0f;
  total_time += deltaTime;

  float rotationSpeed = 100.0f;
  float lightSpeed = 1.0f;

  // Update entities
  for(int i = 0; i < scene->num_entities; i++){
    Entity *entity = &scene->entities[i];

    // Update translation vector
    // entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
    // Update rotation vector
    // entity->rotation[1] -= rotationSpeed * deltaTime;
  }

  // Update light
  scene->light->direction[0] = (float)sin(lightSpeed * total_time);
  //scene->light->direction[1] += y;
  scene->light->direction[2] = (float)cos(lightSpeed * total_time);
}

void scene_render(Scene *scene){
  // Render (clear color and depth buffer bits)
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get view and projection matrices
  mat4 view;
  mat4 projection;
  camera_get_view_matrix(scene->player.camera, view);
  glm_perspective(glm_rad(scene->player.camera->fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

  // Cubemap (move this later, probably to a (optional?) scene property)
  

  // For each entity in the scene
  for(int i = 0; i < scene->num_entities; i++){
    // Bind its shader
    Entity *entity = &scene->entities[i];
    shader_use(entity->shader);

    // Get its model matrix
    mat4 model;
    glm_mat4_identity(model);
    // Translate
    glm_translate(model, entity->position);
    // Rotate
    glm_rotate_y(model, glm_rad(entity->rotation[1]), model);
    glm_rotate_x(model, glm_rad(entity->rotation[0]), model);
    glm_rotate_z(model, glm_rad(entity->rotation[2]), model);
    // Scale
    glm_scale(model, entity->scale);

    // Set normal matrix uniform
    mat3 transposed_mat3;
    mat3 normal;
    glm_mat4_pick3t(model, transposed_mat3);
    glm_mat3_inv(transposed_mat3, normal);
    shader_set_mat3(entity->shader, "normal", normal);

    // Set its model, view, and projection matrix uniforms
    shader_set_mat4(entity->shader, "model", model);
    shader_set_mat4(entity->shader, "view", view);
    shader_set_mat4(entity->shader, "projection", projection);

    // Lighting uniforms
    shader_set_vec3(entity->shader, "dirLight.direction", scene->light->direction);
    shader_set_vec3(entity->shader, "dirLight.ambient", scene->light->ambient);
    shader_set_vec3(entity->shader, "dirLight.diffuse", scene->light->diffuse);
    shader_set_vec3(entity->shader, "dirLight.specular", scene->light->specular);

    // Set camera position as viewPos in the fragment shader
    shader_set_vec3(entity->shader, "viewPos", scene->player.camera->position);

    // Draw model
    model_draw(entity->model, entity->shader);
  }

  float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

  char **faces = malloc(6 * sizeof(char *));
  char *faces[0] = "right.jpg";
  char *faces[1] = "left.jpg";
  char *faces[2] = "top.jpg";
  char *faces[3] = "bottom.jpg";
  char *faces[4] = "front.jpg";
  char *faces[5] = "back.jpg";

  unsigned int skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  GLuint cubemap_ID;
  glGenTextures(1, &cubemap_ID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);

  int width, height, channels;
  for (unsigned int i = 0; i < 6; i++){
    unsigned char *data = stbi_load(faces[i]), &width, &height, &channels, 0);
    if (!data){
      printf("Error: failed to load cubemap face %s\n", faces[i]);
      return 0;
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
  skyboxShader.use();
  view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
  skyboxShader.setMat4("view", view);
  skyboxShader.setMat4("projection", projection);
  // skybox cube
  glBindVertexArray(skyboxVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS); // set depth function back to default
}

void scene_pause(Scene *scene){
  bool prev = scene->paused;
  scene->paused = !prev;
}

void free_scene(Scene *scene){
  if (scene){
    free(scene->entities);
    free(scene->player.camera);
    free(scene->light);
    free(scene);
  }
}
