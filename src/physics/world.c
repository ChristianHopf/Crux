#include "physics/world.h"
#include "narrow_phase.h"
#include "distance.h"
#include "resolution.h"
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
      body->collider = collider;
      glm_vec3_copy(entity->velocity, body->velocity);
      break;
    case false:
      body = &physics_world->static_bodies[physics_world->num_static_bodies++];
      glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, body->velocity);
      break;
  }

  // Check type validity
  if (collider.type < 0 || collider.type > COLLIDER_COUNT){
    fprintf(stderr, "Error: collider type provided to physics_add_body is invalid\n");
    return NULL;
  }

  body->collider = collider;
  glm_vec3_copy(entity->position, body->position);
  glm_vec3_copy(entity->rotation, body->rotation);
  glm_vec3_copy(entity->scale, body->scale);

  return body;
}

void physics_step(struct PhysicsWorld *physics_world, float delta_time){
  if (delta_time > MAX_DELTA_TIME){
    delta_time = MAX_DELTA_TIME;
  }

  // Naive algorithm: check all (dynamic) bodies against all other bodies
  for(unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body_A = &physics_world->dynamic_bodies[i];

    // Collision detection will be off sometimes with AABBs.
    // I think this is because of angular velocity not being considered.
    // This should work fine with spheres, though.
    // body_A->rotation[0] += 100.0f * delta_time;
    // body_A->rotation[0] = fmodf(body_A->rotation[0], 360);
    body_A->rotation[1] += 100.0f * delta_time;
    body_A->rotation[1] = fmodf(body_A->rotation[1], 360);
    // body_A->rotation[2] += 100.0f * delta_time;
    // body_A->rotation[2] = fmodf(body_A->rotation[2], 360);
    // if (body_A->rotation[0] < 0.0f){
    //   body_A->rotation[0] += 360;
    // }
    if (body_A->rotation[1] < 0.0f){
      body_A->rotation[1] += 360;
    }
    // if (body_A->rotation[2] < 0.0f){
    //   body_A->rotation[2] += 360;
    // }

    for (unsigned int j = 0; j < physics_world->num_static_bodies; j++){
      struct PhysicsBody *body_B = &physics_world->static_bodies[j];

      // Order bodies by enum value for function tables
      if (body_A->collider.type > body_B->collider.type){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = body_A;
      }

      // BROAD PHASE: Create a hit_time float and perform interval halving
      float hit_time;
      struct CollisionResult result = {0};
      if (interval_collision(body_A, body_B, 0, delta_time, &hit_time)){
        printf("NARROW PHASE\n");
        // NARROW PHASE
        NarrowPhaseFunction narrow_phase_function = narrow_phase_functions[body_A->collider.type][body_B->collider.type];
        if (!narrow_phase_function){
          fprintf(stderr, "Error: no narrow phase function found for collider types %d, %d\n",
                  body_A->collider.type, body_B->collider.type);
        }
        else{
          result = narrow_phase_function(body_A, body_B, delta_time);
        }
      }

      // COLLISION RESOLUTION
      if (result.colliding && result.hit_time >= 0){
        printf("COLLISION\n");
        ResolutionFunction resolution_function = resolution_functions[body_A->collider.type][body_B->collider.type];
        if (!resolution_function){
          fprintf(stderr, "Error: no collision resolution function found for types %d and %d\n",
                  body_A->collider.type, body_B->collider.type);
        }
        else{
          resolution_function(body_A, body_B, result, delta_time);
        }
      }
    }
    if (!body_A->at_rest){
      float gravity = 9.8f;
      body_A->velocity[1] -= gravity * delta_time;
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
  // print_glm_vec3(body->velocity, "Body velocity");
  // printf("glm_vec3_norm of body velocity: %f\n", glm_vec3_norm(body->velocity));
  return glm_vec3_norm(body->velocity) * delta_time;
}

float minimum_object_distance_at_time(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  // Call the appropriate distance function from the table
  DistanceFunction distance_function = distance_functions[body_A->collider.type][body_B->collider.type];
  return distance_function(body_A, body_B, time);
}
