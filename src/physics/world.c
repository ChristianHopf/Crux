#include "physics/world.h"
#include "distance.h"
#include "utils.h"
#include <cglm/vec3.h>
//#include <signal.h>

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
  world->bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));

  // Might want some kind of default field population later.
  // calloc should be fine for now, though.
  return world;
}

struct PhysicsBody *physics_add_body(struct PhysicsWorld *physics_world, struct Entity *entity){

  // Memory is already allocated: get a pointer, assign values, return the pointer
  struct PhysicsBody *body = &physics_world->bodies[physics_world->num_bodies++];

  // Check type validity

  // Initialize body for the appropriate type
  switch(entity->ID){
    case 1:
      // plane
      struct Plane plane = {
        .normal = {0.0f, 1.0f, 0.0f},
        .distance = 0.0f
      };

      body->type = COLLIDER_PLANE;
      body->collider.plane = plane;
      glm_vec3_copy(entity->position, body->position);
      glm_vec3_copy(entity->rotation, body->rotation);
      glm_vec3_copy(entity->scale, body->scale);
      glm_vec3_copy(entity->velocity, body->velocity);
      // print_glm_vec3(entity->scale, "Entity scale");
      // print_glm_vec3(body->scale, "Copied to body scale");
      break;
    case 2:
      // oiiai
      // struct AABB aabb = {
      //   .center = {0.0014968, 0.1823103, -0.0838950},
      //   .extents = {0.1195973, 0.1933212, 0.3620904}
      // };

      body->type = COLLIDER_AABB;
      // body->collider.aabb = aabb;
      body->collider.aabb = entity->model->aabb;

      // Scale extents assuming uniform scaling, init center at its iniital world space position
      // glm_vec3_scale(body->collider.aabb.extents, entity->scale[0], body->collider.aabb.extents);
      // glm_vec3_mul(body->collider.aabb.center, entity->scale, body->collider.aabb.extents);
      for(int i = 0; i < 3; i++){
        // glm_vec3_scale(body->collider.aabb.extents[i], fabs(entity->scale[i]), body->collider.aabb.extents);
        body->collider.aabb.extents[i] *= fabs(entity->scale[i]);
      }

      // glm_vec3_add(body->collider.aabb.center, entity->position, body->collider.aabb.center);
      // glm_vec3_scale(body->aabb.min, entity->scale[0], body->aabb.min);
      // glm_vec3_scale(body->aabb.max, entity->scale[1], body->aabb.max);

      // Copy the Entity's position, rotation, and velocity
      glm_vec3_copy(entity->position, body->position);
      // print_glm_vec3(entity->position, "Entity position");
      // print_glm_vec3(body->position, "Body position");

      glm_vec3_copy(entity->rotation, body->rotation);
      glm_vec3_copy(entity->scale, body->scale);
      glm_vec3_copy(entity->velocity, body->velocity);
      break;
    // case COLLIDER_PLANE:
    //   return;
    default:
      return;
  }

  // body->type = type;
  return body;
}

void physics_step(struct PhysicsWorld *physics_world, float delta_time){
  if (delta_time > MAX_DELTA_TIME){
    delta_time = MAX_DELTA_TIME;
  }
  // Update each body in the physics world
  // for(unsigned int i = 0; i < physics_world->num_bodies-1; i++){
  //   struct PhysicsBody *body = &physics_world->bodies[i];
  //
  //   // Update velocity according to gravity
  //   float gravity = 9.8f;
  //   body->velocity[1] -= gravity * delta_time;
  //
  //   // Maybe with dynamic testing, we only want to add to its velocity,
  //   // and not yet its position?
  //   glm_vec3_muladds(body->velocity, delta_time, body->position);
  //   print_glm_vec3(body->position, "Applied gravity, new body position");
  // }

  for(unsigned int i = 0; i < physics_world->num_bodies-1; i++){
    struct PhysicsBody *body_A = &physics_world->bodies[i];

    // Get matrix and vector to update current AABB into world space
    mat4 eulerA;
    mat3 rotationA;
    glm_euler_xyz(body_A->rotation, eulerA);
    glm_mat4_pick3(eulerA, rotationA);

    vec3 translationA;
    glm_vec3_copy(body_A->position, translationA);

    // I think this is unnecessary
    vec3 scaleA;
    glm_vec3_copy(body_A->scale, scaleA);

    struct AABB worldAABB_A = {0};
    AABB_update(&body_A->collider.aabb, rotationA, translationA, scaleA, &worldAABB_A);

    // BROAD PHASE:
    // Create a hit_time float and perform interval halving
    printf("BROAD PHASE\n");
    float hit_time;
    if (interval_collision(body_A, &physics_world->bodies[1], 0, delta_time, &hit_time)){
      printf("BROAD PHASE PASSED -> NARROW PHASE\n");
      // NARROW PHASE: AABB against plane

      // Get relative velocity
      vec3 rel_v;
      glm_vec3_copy(body_A->velocity, rel_v);
      // glm_vec3_sub(bodyA->velocity, physics_world->level_plane->velocity, rel_v);

      // Get radius of projection interval
      float r =
        worldAABB_A.extents[0] * fabs(physics_world->bodies[1].collider.plane.normal[0]) +
        worldAABB_A.extents[1] * fabs(physics_world->bodies[1].collider.plane.normal[1]) +
        worldAABB_A.extents[2] * fabs(physics_world->bodies[1].collider.plane.normal[2]); 
      // printf("Got r %f\n", r);

      // Get distance from center of AABB to plane
      float s = glm_dot(physics_world->bodies[1].collider.plane.normal, worldAABB_A.center) - physics_world->bodies[1].collider.plane.distance;
      // printf("DISTANCE FROM AABB CENTER TO PLANE: %f\n", s);

      // Get dot product of normal and relative velocity
      // - n*v = 0 => moving parallel
      // - n*v < 0 => moving towards plane
      // - n*v > 0 => moving away from the plane
      float n_dot_v = glm_dot(physics_world->bodies[1].collider.plane.normal, rel_v);

      // If the distance from the plane is within r, we're already colliding
      if (fabs(s) <= r){
        printf("Already colliding, distance from plane: %.2f\n", s);
        print_aabb(&worldAABB_A);
        hit_time = 0;
      }
      
      // If n*v != 0, solve for t.
      // Ericson's equation:
      // t = (r + d - (n * C)) / (n * v)
      // Is equivalent to:
      // t = (r - ((n * C) - d)) / (n * v), or
      // t = (r - s) / (n * v)
      else if (n_dot_v != 0){
        // Positive side of the plane, moving towards it
        if (s > 0 && n_dot_v < 0){
          // hit_time = (r - s) / -n_dot_v;
          hit_time = (r - s) / n_dot_v;
          printf("COLLISION at t %f\n", hit_time);
        }
        // Negative side of the plane, moving towards it
        else if (s < 0 && n_dot_v > 0){
          // hit_time = (-r - s) / -n_dot_v;
          hit_time = (r - s) / n_dot_v;
        }
      }

      // If t isn't within our given interval, there's no collision within this interval
      if (hit_time < 0 || hit_time > delta_time){
        hit_time = -1;
      }
    }
    // Broad-phase fails: no collision
    else{
      hit_time = -1;
    }

    // COLLISION RESOLUTION
    // If a collision was detected, update position based on hit_time, and update velocity
    if (hit_time != -1){
      glm_vec3_muladds(body_A->velocity, hit_time, body_A->position);
      glm_vec3_scale(body_A->velocity, -1.0f, body_A->velocity);
      print_glm_vec3(body_A->position, "Collision position");
      // bounce off!
    }
    else{
      // Update body position as normal
      float gravity = 9.8f;
      body_A->velocity[1] -= gravity * delta_time;

      // Maybe with dynamic testing, we only want to add to its velocity,
      // and not yet its position?
      glm_vec3_muladds(body_A->velocity, delta_time, body_A->position);
      print_glm_vec3(body_A->position, "Applied gravity, new body position");
    }
  }

  // Perform primitive collision detection:
  // a single broad-phase check of every possible pair
  // for(unsigned int i = 0; i < physics_world->num_bodies; i++){
  //   struct PhysicsBody *bodyA = &physics_world->bodies[i];
  //
  //   // Get matrix and vector to update current AABB into world space
  //   mat4 eulerA;
  //   mat3 rotationA;
  //   glm_euler_xyz(bodyA->rotation, eulerA);
  //   glm_mat4_pick3(eulerA, rotationA);
  //
  //   vec3 translationA;
  //   glm_vec3_copy(bodyA->position, translationA);
  //
  //   struct AABB worldAABB_A = {0};
  //   AABB_update(&bodyA->aabb, rotationA, translationA, &worldAABB_A);

    // Check collision with world space AABB and level plane
  //   printf("Time to check collision between a world space AABB and the level plane!\n");
  //   printf("World space AABB:\n");
  //   print_aabb(&worldAABB_A);
  //   printf("physics_world->level_plane:\n");
  //   print_plane_collider(physics_world->level_plane);
  //   if (AABB_intersect_plane(&worldAABB_A, physics_world->level_plane)){
  //     printf("COLLISION DETECTED\n");
  //     // Primitive collision resolution: apply impulse to reverse the body's velocity
  //     glm_vec3_scale(bodyA->velocity, -1.0f, bodyA->velocity);
  //     print_glm_vec3(bodyA->velocity, "New oiiai velocity");
  //   }
  //   else{
  //     printf("NO COLLISION DETECTED\n");
  //   }
  //
  //   // Check for collision with every other entity
  //   for(unsigned int j = i+1; j < physics_world->num_bodies; j++){
  //     struct PhysicsBody *bodyB = &physics_world->bodies[j];
  //     // Get matrix and vector to update AABB A
  //     mat4 eulerB;
  //     mat3 rotationB;
  //     glm_euler_xyz(bodyB->rotation, eulerB);
  //     glm_mat4_pick3(eulerB, rotationB);
  //
  //     vec3 translationB;
  //     glm_vec3_copy(bodyB->position, translationB);
  //
  //     struct AABB worldAABB_B = {0};
  //     AABB_update(&bodyB->aabb, rotationB, translationB, &worldAABB_B);
  //
  //     // Perform collision check
  //     printf("Time to check collision between the following AABBs:\n");
  //     printf("Model space AABBs:\n");
  //     print_aabb(&bodyA->aabb);
  //     print_aabb(&bodyB->aabb);
  //     printf("World space AABBS:\n");
  //     print_aabb(&worldAABB_A);
  //     print_aabb(&worldAABB_B);
  //     if (AABB_intersect_AABB(&worldAABB_A, &worldAABB_B)){
  //       printf("Collision detected\n");
  //     }
  //     else{
  //       printf("No collision detected\n");
  //     }
  //   }
  //
  //   // Update translation vector
  //   // entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
  //   // Update rotation vector
  //   // entity->rotation[1] -= rotationSpeed * deltaTime;
  // }
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

  switch(body->type){
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
  DistanceFunction distance_function = distance_functions[body_A->type][body_B->type];
  return distance_function(body_A, body_B, time);
}

// Refactor into some pretty enum solution later
// float maximum_object_movement_over_time_aabb(struct PhysicsBody *body, float start_time, float end_time){
//   float interval = end_time - start_time;
//   return glm_vec3_norm(body->velocity) * interval;
// }

// In the future, planes will be bounded and can move... maybe
// float maximum_object_movement_over_time_plane(struct PlaneCollider *plane, float start_time, float end_time){
//   return 0.0f;
// }

// For now, only an AABB and a plane
// float minimum_object_distance_at_time(struct PhysicsBody *body, struct PlaneCollider *plane, float time){
//
//   // Get world space AABB to find minimum distance
//   mat4 eulerA;
//   mat3 rotationA;
//   glm_euler_xyz(body->rotation, eulerA);
//   glm_mat4_pick3(eulerA, rotationA);
//
//   vec3 translationA;
//   glm_vec3_copy(body->position, translationA);
//   glm_vec3_muladds(body->velocity, time, translationA);
//
//   struct AABB worldAABB_A = {0};
//   AABB_update(&body->aabb, rotationA, translationA, &worldAABB_A);
//
//   // Get radius of the extents' projection interval onto the plane's normal
//   float r =
//     worldAABB_A.extents[0] * fabs(plane->normal[0]) +
//     worldAABB_A.extents[1] * fabs(plane->normal[1]) +
//     worldAABB_A.extents[2] * fabs(plane->normal[2]); 
//   // Get distance from the center of AABB to the plane
//   float s = glm_dot(plane->normal, worldAABB_A.center) - plane->distance;
//
//   return glm_max(s - r, 0);
// }
