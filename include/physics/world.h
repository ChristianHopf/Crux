#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include <cglm/cglm.h>
#include "physics/aabb.h"
#include "physics/utils.h"
#include "entity.h"

struct PhysicsBody {
  struct AABB aabb;
  vec3 position;
  vec3 rotation;
  vec3 velocity;
};

struct PhysicsWorld {
  struct PhysicsBody *bodies;
  unsigned int num_bodies;
  struct PlaneCollider *level_plane;
};


// World, bodies
struct PhysicsWorld *physics_world_create();
struct PhysicsBody *physics_add_body(struct PhysicsWorld *physics_world, struct Entity *entity);

void physics_step(struct PhysicsWorld *physics_world, float delta_time);

bool interval_collision(struct PhysicsBody *body, struct PlaneCollider *plane, float start_time, float end_time, float *hit_time);
float maximum_object_movement_over_time_aabb(struct PhysicsBody *body, float start_time, float end_time);
float maximum_object_movement_over_time_plane(struct PlaneCollider *plane, float start_time, float end_time);
float minimum_object_distance_at_time(struct PhysicsBody *body, struct PlaneCollider *plane, float time);

#endif
