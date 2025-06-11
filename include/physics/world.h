#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include "physics/aabb.h"

struct PhysicsBody {
  struct AABB aabb;
};

struct PhysicsWorld {
  struct PhysicsBody *bodies;
  unsigned int num_bodies;
};


// World, bodies
struct PhysicsWorld *physics_world_create();
void physics_add_body(struct PhysicsWorld *physics_world, struct AABB aabb);

void physics_step(struct PhysicsWorld *physics_world, float delta_time);

#endif
