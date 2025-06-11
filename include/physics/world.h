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

#endif
