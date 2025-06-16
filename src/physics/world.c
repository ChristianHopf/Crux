#include "physics/world.h"
#include "narrow_phase.h"
#include "distance.h"
#include "utils.h"
#include <cglm/vec3.h>

#define MAX_PHYSICS_BODIES 128

// Fixed time step to avoid huge delta time values
#define MAX_DELTA_TIME 0.01666

// Collision timing
#define INTERVAL_EPSILON 0.000001f

struct PhysicsWorld *physics_world_create(){
  struct PhysicsWorld *world = (struct PhysicsWorld *)calloc(1, sizeof(struct PhysicsWorld));
  if (!world){
    printf("Error: failed to allocate PhysicsWorld in physics_world_create\n");
    return NULL;
  }
  // Good opportunity for optimization here:
  // - each bodies array should only have enough memory allocated for initial loading
  // - if a static body becomes dynamic, or vice versa, reallocate accordingly
  world->static_bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));
  world->dynamic_bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));

  // Might want some kind of default field population later.
  // calloc should be fine for now, though.
  return world;
}

struct PhysicsBody *physics_add_body(struct PhysicsWorld *physics_world, struct Entity *entity, struct Collider collider, bool dynamic){

  // Memory is already allocated: get a pointer, assign values, return the pointer
  struct PhysicsBody *body;
  switch(dynamic){
    case true:
      body = &physics_world->dynamic_bodies[physics_world->num_dynamic_bodies++];
      break;
    case false:
      body = &physics_world->static_bodies[physics_world->num_static_bodies++];
      break;
  }

  // Check type validity

  // Initialize body for the appropriate type
  switch(collider.type){
    case COLLIDER_AABB:
      body->collider.data.aabb = collider.data.aabb;

      glm_vec3_copy(entity->position, body->position);
      glm_vec3_copy(entity->rotation, body->rotation);
      glm_vec3_copy(entity->scale, body->scale);
      glm_vec3_copy(entity->velocity, body->velocity);
      break;
    case COLLIDER_PLANE:
      body->collider.data.plane = collider.data.plane;

      glm_vec3_copy(entity->position, body->position);
      glm_vec3_copy(entity->rotation, body->rotation);
      glm_vec3_copy(entity->scale, body->scale);
      glm_vec3_copy(entity->velocity, body->velocity);
      break;
    default:
      return NULL;
  }

  body->collider.type = collider.type;
  return body;
}

void physics_step(struct PhysicsWorld *physics_world, float delta_time){
  if (delta_time > MAX_DELTA_TIME){
    delta_time = MAX_DELTA_TIME;
  }

  for(unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body_A = &physics_world->dynamic_bodies[i];

    // BROAD PHASE:
    // Create a hit_time float and perform interval halving
    printf("BROAD PHASE\n");
    float hit_time;
    struct CollisionResult result = {0};
    if (interval_collision(body_A, &physics_world->static_bodies[0], 0, delta_time, &hit_time)){
      printf("BROAD PHASE PASSED -> NARROW PHASE\n");
      // NARROW PHASE: AABB against plane
      result = narrow_phase_AABB_plane(body_A, &physics_world->static_bodies[0], delta_time);
    }

    // COLLISION RESOLUTION
    // If a collision was detected, update position based on hit_time, and update velocity
    if (result.colliding && result.hit_time >= 0){
      // First move by velocity according to hit_time, applying gravity until collision
      float gravity = 9.8f;
      body_A->velocity[1] -= 9.8 * result.hit_time;
      glm_vec3_muladds(body_A->velocity, result.hit_time, body_A->position);
      // glm_vec3_scale(body_A->velocity, -1.0f, body_A->velocity);

      // Correct position in case of penetration
      if (result.penetration){
        glm_vec3_muladds(physics_world->static_bodies[0].collider.data.plane.normal, result.penetration, body_A->position);
      }

      // Reflect velocity vector over normal
      float v_dot_n = glm_dot(body_A->velocity, physics_world->static_bodies[0].collider.data.plane.normal);
      vec3 reflection;
      glm_vec3_scale(physics_world->static_bodies[0].collider.data.plane.normal, -2.0f * v_dot_n, reflection);
      glm_vec3_add(body_A->velocity, reflection, body_A->velocity);

      // Update body position as normal, with remaining time
      float remaining_time = delta_time - result.hit_time;
      if (remaining_time > 0){
        body_A->velocity[1] -= gravity * remaining_time;

        glm_vec3_muladds(body_A->velocity, remaining_time, body_A->position);
      }

      print_glm_vec3(body_A->position, "New body position");
      print_glm_vec3(body_A->velocity, "New body velocity");
    }
    else{
      // Update body position as normal
      float gravity = 9.8f;
      body_A->velocity[1] -= gravity * delta_time;

      printf("No collision, updating position with gravity\n");
      glm_vec3_muladds(body_A->velocity, delta_time, body_A->position);
    }
  }
}

bool interval_collision(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float start_time, float end_time, float *hit_time){
  // Compute sum of maximum distances the two bodies move over the interval
  float max_move_A = maximum_object_movement_over_time(body_A, start_time, end_time);
  float max_move_B = maximum_object_movement_over_time(body_B, start_time, end_time);
  float max_move_sum = max_move_A + max_move_B;

  // If initial minimum distance is larger than max_move_sum, exit
  float min_dist_start = minimum_object_distance_at_time(body_A, body_B, start_time);
  if (min_dist_start > max_move_sum) return 0;

  // If end minimum distance is still larger than max_move_sum, exit (bodies are moving away)
  float min_dist_end = minimum_object_distance_at_time(body_A, body_B, end_time);
  if (min_dist_end > max_move_sum) return 0;

  // If we recurse down to a small enough interval, assume collision at the start of the interval
  if (end_time - start_time < INTERVAL_EPSILON){
    *hit_time = start_time;
    return 1;
  }

  // Collision may have happened: halve the interval and check each half
  float mid_time = (start_time + end_time) * 0.5f;
  if (interval_collision(body_A, body_B, start_time, mid_time, hit_time)) return 1;

  return interval_collision(body_A, body_B, mid_time, end_time, hit_time);
}

float maximum_object_movement_over_time(struct PhysicsBody *body, float start_time, float end_time){
  float delta_time = end_time - start_time;

  switch(body->collider.type){
    case COLLIDER_AABB:
      return glm_vec3_norm(body->velocity) * delta_time;
    case COLLIDER_PLANE:
      return 0.0f;
    default:
      return 0.0f;
  }
}

float minimum_object_distance_at_time(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  // Call the appropriate distance function from the table
  DistanceFunction distance_function = distance_functions[body_A->collider.type][body_B->collider.type];
  return distance_function(body_A, body_B, time);
}
