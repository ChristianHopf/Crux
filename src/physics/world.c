#include "physics/world.h"

#define MAX_PHYSICS_BODIES 128

struct PhysicsWorld *physics_world_create(){
  struct PhysicsWorld *world = (struct PhysicsWorld *)calloc(1, sizeof(struct PhysicsWorld));
  if (!world){
    printf("Error: failed to allocate PhysicsWorld in physics_world_create\n");
    return NULL;
  }
  world->bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));
  printf("Created physics world\n");

  // Might want some kind of default field population later.
  // calloc should be fine for now, though.
  return world;
}

void physics_add_body(struct PhysicsWorld *physics_world, Entity *entity){
  printf("Physics world has %d bodies\n", physics_world->num_bodies);
  // Create body
  // struct PhysicsBody *body = (struct PhysicsBody *)malloc(sizeof(struct PhysicsBody));
  // body->aabb = *aabb;
  // printf("Created body with AABB:\n");
  // print_aabb(&body->aabb);

  // Add body to physics_world
  // physics_world->bodies[physics_world->num_bodies++] = *body;

  struct PhysicsBody body = {
    .aabb = entity->model->aabb,
  };
  glm_vec3_copy(body.position, entity->position);
  glm_vec3_copy(body.rotation, entity->rotation);
  glm_vec3_copy(body.velocity, entity->velocity);

  printf("Successfully created physics body with AABB:\n");
  print_aabb(&body.aabb);

  physics_world->bodies[physics_world->num_bodies++] = body;
  printf("Allocated physics body, physics world now has %d bodies\n", physics_world->num_bodies);
  // physics_world->bodies[physics_world->num_bodies].aabb = aabb;
  // printf("Successfully added body to physics_world\n");
}

void physics_step(struct PhysicsWorld *physics_world, float delta_time){
  // Perform primitive collision detection:
  // a single broad-phase check of every possible pair
  for(unsigned int i = 0; i < physics_world->num_bodies-1; i++){
    struct PhysicsBody *bodyA = &physics_world->bodies[i];

    // Get matrix and vector to update current AABB into world space
    mat4 eulerA;
    mat3 rotationA;
    glm_euler_xyz(bodyA->rotation, eulerA);
    glm_mat4_pick3(eulerA, rotationA);
      
    vec3 translationA;
    glm_vec3_copy(bodyA->position, translationA);
      
    struct AABB worldAABB_A = {0};
    AABB_update(&bodyA->aabb, rotationA, translationA, &worldAABB_A);

    // Check for collision with every other entity
    for(int j = i+1; j < physics_world->num_bodies; j++){
      struct PhysicsBody *bodyB = &physics_world->bodies[j];
      // Get matrix and vector to update AABB A
      mat4 eulerB;
      mat3 rotationB;
      glm_euler_xyz(bodyB->rotation, eulerB);
      glm_mat4_pick3(eulerB, rotationB);
      
      vec3 translationB;
      glm_vec3_copy(bodyB->position, translationB);
      
      struct AABB worldAABB_B = {0};
      AABB_update(&bodyB->aabb, rotationB, translationB, &worldAABB_B);

      // Perform collision check
      if (AABB_intersect(&worldAABB_A, &worldAABB_B)){
        // printf("1\n");
        printf("Collision detected between the following aabbs:\n");
        print_aabb(&worldAABB_A);
        print_aabb(&worldAABB_B);
      }
      else{
        // printf("2\n");
        // printf("No collision detected between the following aabbs:\n");
        // print_aabb(&worldAABB_A);
        // print_aabb(&worldAABB_B);
      }
    }

    // Update translation vector
    // entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
    // Update rotation vector
    // entity->rotation[1] -= rotationSpeed * deltaTime;
  }
}
