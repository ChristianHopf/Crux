#include "physics/world.h"
#include "narrow_phase.h"
#include "distance.h"
#include "resolution.h"
#include "event.h"
#include "utils.h"
#include <cglm/vec3.h>
#include <stdbool.h>
#include "time.h"

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
  world->player_bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));

  // Might want some kind of default field population later.
  // calloc should be fine for now, though.
  return world;
}

struct PhysicsBody *physics_add_body(struct PhysicsWorld *physics_world, struct SceneNode *scene_node, struct Entity *entity, struct Collider collider, float restitution, bool dynamic){
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

  // Get position, rotation, and scale from world transform
  vec3 world_position, world_rotation, world_scale;
  glm_mat4_mulv3(scene_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position);
  glm_decompose_scalev(scene_node->world_transform, world_scale);
  mat3 rotation_mat3;
  glm_mat4_pick3(scene_node->world_transform, rotation_mat3);
  if (world_scale[0] != 0.0f){
    glm_mat3_scale(rotation_mat3, 1.0f / world_scale[0]);
  }

  vec3 euler_radians, euler_degrees;
  
  // Assuming rotation_mat is normalized (scale removed)
  // XYZ order: Rx * Ry * Rz
  // rotation_mat = [r11 r12 r13]
  //               [r21 r22 r23]
  //               [r31 r32 r33]
  float r11 = rotation_mat3[0][0];
  float r12 = rotation_mat3[0][1];
  float r13 = rotation_mat3[0][2];
  float r21 = rotation_mat3[1][0];
  float r31 = rotation_mat3[2][0];
  float r32 = rotation_mat3[2][1];
  float r33 = rotation_mat3[2][2];

  // Pitch (Y) = asin(-r31)
  euler_radians[1] = asinf(-r31); // -pi/2 to pi/2

  // Handle gimbal lock cases (r31 ≈ ±1)
  if (fabsf(r31) > 0.9999f) {
    // Gimbal lock: pitch is ±90 degrees
    euler_radians[0] = 0.0f; // Roll (X) is undefined, set to 0
    euler_radians[2] = atan2f(r12, r11); // Yaw (Z)
  } else {
    // Roll (X) = atan2(r32, r33)
    euler_radians[0] = -atan2f(r32, r33);
    // Yaw (Z) = atan2(r21, r11)
    euler_radians[2] = -atan2f(r21, r11);
  }

  // Convert to degrees
  euler_degrees[0] = glm_deg(euler_radians[0]); // Roll (X)
  euler_degrees[1] = glm_deg(euler_radians[1]); // Pitch (Y)
  euler_degrees[2] = glm_deg(euler_radians[2]); // Yaw (Z)


  // glm_vec3_copy(world_position, body->position);
  // glm_vec3_copy(euler_degrees, body->rotation);
  // glm_vec3_copy(world_scale, body->scale);

  glm_vec3_copy(entity->position, body->position);
  glm_vec3_copy(entity->rotation, body->rotation);
  glm_vec3_copy(entity->scale, body->scale);
  body->collider = collider;
  body->restitution = restitution;
  body->entity = entity;
  body->scene_node = scene_node;

  return body;
}

struct PhysicsBody *physics_add_player(struct PhysicsWorld *physics_world, struct Entity *entity, struct Collider collider){
  // Check type validity
  if (collider.type < 0 || collider.type > COLLIDER_COUNT){
    fprintf(stderr, "Error: collider type provided to physics_add_body is invalid\n");
    return NULL;
  }
  struct PhysicsBody *body;
  body = &physics_world->player_bodies[physics_world->num_player_bodies++];

  glm_vec3_copy(entity->position, body->position);
  glm_vec3_copy(entity->rotation, body->rotation);
  glm_vec3_copy(entity->scale, body->scale);
  glm_vec3_copy(entity->velocity, body->velocity);
  body->collider = collider;
  body->restitution = 0.0f;
  body->entity = entity;
  body->scene_node = NULL;

  return body;
}

void physics_step(struct PhysicsWorld *physics_world, float delta_time){
  if (delta_time > MAX_DELTA_TIME){
    delta_time = MAX_DELTA_TIME;
  }

  for (unsigned int i = 0; i < physics_world->num_player_bodies; i++){
    struct PhysicsBody *body_A = &physics_world->player_bodies[i];

    for (unsigned int j = 0; j < physics_world->num_static_bodies; j++){
      struct PhysicsBody *body_B = &physics_world->static_bodies[j];

      // Order bodies by enum value for function tables
      bool body_swap = false;
      if (body_A->collider.type > body_B->collider.type){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = temp;
        body_swap = true;
      }

      // BROAD PHASE: Create a hit_time float and perform interval halving
      float hit_time;
      struct CollisionResult result = {0};
      if (interval_collision(body_A, body_B, 0, delta_time, &hit_time)){
        // NARROW PHASE
        NarrowPhaseFunction narrow_phase_function = narrow_phase_functions[body_A->collider.type][body_B->collider.type];
        if (!narrow_phase_function){
          fprintf(stderr, "Error: no narrow phase function found for collider types %d, %d\n", body_A->collider.type, body_B->collider.type);
        }
        else{
          result = narrow_phase_function(body_A, body_B, delta_time);
        }
      }

      // COLLISION RESOLUTION
      if (result.colliding && result.hit_time >= 0){
        // Determine resolution strategy
        EntityType type_A = body_A->entity->type;
        EntityType type_B = body_B->entity->type;
        CollisionBehavior behavior = get_collision_behavior(type_A, type_B);

        // Get appropriate resolution function for the given CollisionBehavior
        switch(behavior){
          case COLLISION_BEHAVIOR_PHYSICS:
            ResolutionFunction resolution_function = resolution_functions[body_A->collider.type][body_B->collider.type];
            if (!resolution_function){
              fprintf(stderr, "Error: no collision resolution function found for types %d, %d\n", body_A->collider.type, body_B->collider.type);
            }
            else{
              resolution_function(body_A, body_B, result, delta_time);
            }
            break;
          case COLLISION_BEHAVIOR_TRIGGER:
            break;
        }

        // Generate Event
        struct GameEvent event;
        struct timespec timestamp;
        if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1){
          perror("clock_gettime");
          timestamp.tv_nsec = 0;
        }
        event.timestamp = timestamp;

        // TODO refactor to a more robust soln that determines which is the item.
        // This works in my case because a Player is always a capsule,
        // and an item will always be an AABB.
        // (We CAN still assume one of the bodies is a Player because we're looping over player_bodies here.)
        event.type = get_event_type(type_A, type_B);
        switch(event.type){
          case EVENT_COLLISION:
            event.data.collision.entity_A = 0;
            event.data.collision.entity_B = 0;
            break;
          case EVENT_PLAYER_ITEM_PICKUP:
            // TODO player ID, physicsbody/world has knowledge of it somehow
            memcpy(event.data.item_pickup.player_entity_id, body_B->entity->id, 16);
            // event.data.item_pickup.player_entity_id = body_B->entity->id;
            event.data.item_pickup.item_id = body_A->entity->item->id;
            event.data.item_pickup.item_count = body_A->entity->item->count;
            memcpy(event.data.item_pickup.item_entity_id, body_A->entity->id, 16);
            break;
        }

        game_event_queue_enqueue(event);
      }

      // Reorder bodies by enum value
      if (body_swap){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = temp;
      }
    }
    if (!body_A->at_rest){
      float gravity = 9.8f;
      body_A->velocity[1] -= gravity * delta_time;
      glm_vec3_muladds(body_A->velocity, delta_time, body_A->position);
    }
  }

  for(unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body_A = &physics_world->dynamic_bodies[i];

    // Collision detection will be off sometimes with AABBs.
    // I think this is because of angular velocity not being considered.
    // This should work fine with spheres, though.
    // body_A->rotation[0] += 100.0f * delta_time;
    // body_A->rotation[0] = fmodf(body_A->rotation[0], 360);
    // body_A->rotation[1] += 100.0f * delta_time;
    // body_A->rotation[1] = fmodf(body_A->rotation[1], 360);
    // body_A->rotation[2] += 100.0f * delta_time;
    // body_A->rotation[2] = fmodf(body_A->rotation[2], 360);
    // if (body_A->rotation[0] < 0.0f){
    //   body_A->rotation[0] += 360;
    // }
    // if (body_A->rotation[1] < 0.0f){
    //   body_A->rotation[1] += 360;
    // }
    // if (body_A->rotation[2] < 0.0f){
    //   body_A->rotation[2] += 360;
    // }

    for (unsigned int j = 0; j < physics_world->num_static_bodies; j++){
      struct PhysicsBody *body_B = &physics_world->static_bodies[j];

      // Order bodies by enum value for function tables
      bool body_swap = false;
      if (body_A->collider.type > body_B->collider.type){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = temp;
        body_swap = true;
      }

      // BROAD PHASE: Create a hit_time float and perform interval halving
      float hit_time;
      struct CollisionResult result = {0};
      if (interval_collision(body_A, body_B, 0, delta_time, &hit_time)){
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
        // Determine resolution strategy
        EntityType type_A = body_A->entity->type;
        EntityType type_B = body_B->entity->type;
        CollisionBehavior behavior = get_collision_behavior(type_A, type_B);

        // Get appropriate resolution function for the given CollisionBehavior
        switch(behavior){
          case COLLISION_BEHAVIOR_PHYSICS:
            ResolutionFunction resolution_function = resolution_functions[body_A->collider.type][body_B->collider.type];
            if (!resolution_function){
              fprintf(stderr, "Error: no collision resolution function found for types %d, %d\n", body_A->collider.type, body_B->collider.type);
            }
            else{
              resolution_function(body_A, body_B, result, delta_time);
            }
            break;
          case COLLISION_BEHAVIOR_TRIGGER:
            break;
        }

        // Generate Event
        struct GameEvent event;
        struct timespec timestamp;
        if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1){
          perror("clock_gettime");
          timestamp.tv_nsec = 0;
        }
        event.timestamp = timestamp;
        event.type = get_event_type(type_A, type_B);

        switch(event.type){
          case EVENT_COLLISION:
            event.data.collision.entity_A = 0;
            event.data.collision.entity_B = 0;
            break;
        }

        game_event_queue_enqueue(event);
      }
      // Reorder bodies by enum value
      if (body_swap){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = temp;
      }
    }
    // if (!body_A->at_rest){
    //   float gravity = 9.8f;
    //   body_A->velocity[1] -= gravity * delta_time;
    //   glm_vec3_muladds(body_A->velocity, delta_time, body_A->position);
    // }
  }

  for(unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body_A = &physics_world->dynamic_bodies[i];

    for(unsigned int j = 0; j < physics_world->num_dynamic_bodies; j++){
      if (i == j) continue;

      struct PhysicsBody *body_B = &physics_world->dynamic_bodies[j];

      // Order bodies by enum value for function tables
      bool body_swap = false;
      if (body_A->collider.type > body_B->collider.type){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = temp;
        body_swap = true;
      }

      float hit_time;
      struct CollisionResult result = {0};
      if (interval_collision(body_A, body_B, 0, delta_time, &hit_time)){
        NarrowPhaseFunction narrow_phase_function = narrow_phase_functions[body_A->collider.type][body_B->collider.type];
        if (!narrow_phase_function){
        fprintf(stderr, "Error: no narrow phase function found for collider types %d, %d\n",
                  body_A->collider.type, body_B->collider.type);
        }
        else{
          result = narrow_phase_function(body_A, body_B, delta_time);
        }
      }
      if (result.colliding && result.hit_time >= 0){
        ResolutionFunction resolution_function = resolution_functions[body_A->collider.type][body_B->collider.type];
        if (!resolution_function){
          fprintf(stderr, "Error: no collision resolution function found for types %d and %d\n",
                  body_A->collider.type, body_B->collider.type);
        }
        else{
          resolution_function(body_A, body_B, result, delta_time);

          struct timespec timestamp;
          EventType event_type = EVENT_COLLISION;
          if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1){
            perror("clock_gettime");
            timestamp.tv_nsec = 0;
          }
          if (body_A->entity->type == ENTITY_ITEM){
            event_type = EVENT_PLAYER_ITEM_PICKUP;
          }
          struct GameEvent collision = {
            .type = EVENT_COLLISION,
            .timestamp = timestamp,
            .data.collision = {
              .entity_A = 0,
              .entity_B = 1
            }
          };
          game_event_queue_enqueue(collision);
        }
      }
      // Reorder bodies by enum value
      if (body_swap){
        struct PhysicsBody *temp = body_A;
        body_A = body_B;
        body_B = temp;
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
  if (!distance_function){
    fprintf(stderr, "Error: no minimum distance function found for types %d and %d\n",
            body_A->collider.type, body_B->collider.type);
    return 0;
  }
  return distance_function(body_A, body_B, time);
}
