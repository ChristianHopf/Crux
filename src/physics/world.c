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
    if (result.colliding && result.hit_time >= 0){
      // First move by velocity according to hit_time, applying gravity until collision
      float gravity = 9.8f;
      vec3 velocity_before;
      glm_vec3_copy(body_A->velocity, velocity_before);
      velocity_before[1] -= gravity * result.hit_time;
      glm_vec3_muladds(velocity_before, result.hit_time, body_A->position);



      // glm_vec3_copy(result.point_of_contact, body_A->position);
      // glm_vec3_scale(body_A->velocity, -1.0f, body_A->velocity);
      
      // Correct position in case of penetration
      // if (result.penetration){
      //   glm_vec3_muladds(physics_world->static_bodies[0].collider.data.plane.normal, result.penetration, body_A->position);
      // }

      struct AABB worldAABB = {0};
      mat4 eulerA;
      mat3 rotationA;
      glm_euler_xyz(body_A->rotation, eulerA);
      glm_mat4_pick3(eulerA, rotationA);
      vec3 translationA, scaleA;
      glm_vec3_copy(body_A->position, translationA);
      glm_vec3_copy(body_A->scale, scaleA);
      AABB_update(&body_A->collider.data.aabb, rotationA, translationA, scaleA, &worldAABB);
      vec3 normal;
      glm_vec3_copy(physics_world->static_bodies[0].collider.data.plane.normal, normal);
      float r = worldAABB.extents[0] * fabsf(normal[0]) +
                worldAABB.extents[1] * fabsf(normal[1]) +
                worldAABB.extents[2] * fabsf(normal[2]);
      float s = glm_vec3_dot(normal, worldAABB.center) - physics_world->static_bodies[0].collider.data.plane.distance;
      float penetration = (s < r) ? (r - s) : 0.0f; // Only correct if penetrating
      if (penetration > 0.0f) {
        vec3 correction;
        glm_vec3_scale(normal, penetration, correction);
        print_glm_vec3(body_A->position, "Body position before correction");
        glm_vec3_add(body_A->position, correction, body_A->position);
        printf("Penetration correction: %f\n", penetration);
        print_glm_vec3(body_A->position, "Corrected body position");
        print_glm_vec3(body_A->collider.data.aabb.extents, "Body extents");
      }

      // Reflect velocity vector over normal
      float restitution = 0.8f;
      float rest_velocity_threshold = 0.1f;
      float v_dot_n = glm_dot(velocity_before, physics_world->static_bodies[0].collider.data.plane.normal);
      vec3 reflection;
      glm_vec3_scale(physics_world->static_bodies[0].collider.data.plane.normal, -2.0f * v_dot_n * restitution, reflection);
      glm_vec3_add(velocity_before, reflection, body_A->velocity);
      print_glm_vec3(body_A->velocity, "Reflected body velocity after penetration correction");

      float velocity_magnitude = glm_vec3_norm(body_A->velocity);
      if (velocity_magnitude < 0.5){
printf("TINY VELOCITY MAGNITUDE: %f\n", velocity_magnitude);
        float distance_to_plane = glm_dot(worldAABB.center, normal) - physics_world->static_bodies[0].collider.data.plane.distance;
        printf("Distance to plane: %f\n", distance_to_plane);
        print_glm_vec3(worldAABB.extents, "World AABB extents");
      }
      


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
      print_glm_vec3(body_A->position, "New body position");
      print_glm_vec3(body_A->velocity, "New body velocity");
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
