#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include <cglm/cglm.h>
#include "physics/aabb.h"
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
};


// World, bodies
struct PhysicsWorld *physics_world_create();
void physics_add_body(struct PhysicsWorld *physics_world, struct Entity *entity);

void physics_step(struct PhysicsWorld *physics_world, float delta_time);

#endif
