#include "physics/world.h"
#include "utils.h"
//#include <signal.h>

#define MAX_PHYSICS_BODIES 128

struct PhysicsWorld *physics_world_create(){
  struct PhysicsWorld *world = (struct PhysicsWorld *)calloc(1, sizeof(struct PhysicsWorld));
  if (!world){
    printf("Error: failed to allocate PhysicsWorld in physics_world_create\n");
    return NULL;
  }
  world->bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));

  // Might want some kind of default field population later.
  // calloc should be fine for now, though.
  return world;
}

struct PhysicsBody *physics_add_body(struct PhysicsWorld *physics_world, struct Entity *entity){

  // Memory is already allocated: get a pointer, assign values, return the pointer
  struct PhysicsBody *body = &physics_world->bodies[physics_world->num_bodies++];

  // Assume uniform scaling
  body->aabb = entity->model->aabb;
  glm_vec3_scale(body->aabb.extents, entity->scale[0], body->aabb.extents);
  // glm_vec3_scale(body->aabb.min, entity->scale[0], body->aabb.min);
  // glm_vec3_scale(body->aabb.max, entity->scale[1], body->aabb.max);

  // Copy the Entity's position, rotation, and velocity
  glm_vec3_copy(entity->position, body->position);
  glm_vec3_copy(entity->rotation, body->rotation);
  glm_vec3_copy(entity->velocity, body->velocity);

  return body;
}

void physics_step(struct PhysicsWorld *physics_world, float delta_time){
  // Update each body in the physics world
  for(unsigned int i = 0; i < physics_world->num_bodies; i++){
    struct PhysicsBody *body = &physics_world->bodies[i];

    // Update velocity according to gravity
    float gravity = 2.0f * delta_time;
    body->velocity[1] -= gravity;

    glm_vec3_muladds(body->velocity, delta_time, body->position);
  }

  // Perform primitive collision detection:
  // a single broad-phase check of every possible pair
  for(unsigned int i = 0; i < physics_world->num_bodies; i++){
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

    // Check collision with world space AABB and level plane
    printf("Time to check collision between a world space AABB and the level plane!\n");
    printf("World space AABB:\n");
    print_aabb(&worldAABB_A);
    printf("physics_world->level_plane:\n");
    print_plane_collider(physics_world->level_plane);
    if (AABB_intersect_plane(&worldAABB_A, physics_world->level_plane)){
      printf("COLLISION DETECTED\n");
      // Primitive collision resolution: apply impulse to reverse the body's velocity
      glm_vec3_scale(bodyA->velocity, -1.0f, bodyA->velocity);
      print_glm_vec3(bodyA->velocity, "New oiiai velocity");
    }
    else{
      printf("NO COLLISION DETECTED\n");
    }

    // Check for collision with every other entity
    for(unsigned int j = i+1; j < physics_world->num_bodies; j++){
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
      printf("Time to check collision between the following AABBs:\n");
      printf("Model space AABBs:\n");
      print_aabb(&bodyA->aabb);
      print_aabb(&bodyB->aabb);
      printf("World space AABBS:\n");
      print_aabb(&worldAABB_A);
      print_aabb(&worldAABB_B);
      if (AABB_intersect_AABB(&worldAABB_A, &worldAABB_B)){
        printf("Collision detected\n");
      }
      else{
        printf("No collision detected\n");
      }
    }

    // Update translation vector
    // entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
    // Update rotation vector
    // entity->rotation[1] -= rotationSpeed * deltaTime;
  }
}
