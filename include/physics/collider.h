#pragma once

#include <cglm/cglm.h>
#include "aabb.h"
#include "sphere.h"
#include "capsule.h"
#include "plane.h"
#include "types.h"

// Enum for resolution strategy for colliding PhysicsBodies
// - COLLISION_BEHAVIOR_PHYSICS: physics-based collision resolution
// - COLLISION_BEHAVIOR_TRIGGER: simply trigger an event, no physics necessary
typedef enum {
  COLLISION_BEHAVIOR_PHYSICS = 0,
  COLLISION_BEHAVIOR_TRIGGER
} CollisionBehavior;

typedef enum {
  COLLIDER_AABB = 0,
  COLLIDER_SPHERE,
  COLLIDER_CAPSULE,
  COLLIDER_PLANE,
  COLLIDER_COUNT
} ColliderType;

union ColliderData {
  struct AABB aabb;
  struct Sphere sphere;
  struct Capsule capsule;
  struct Plane plane;
};

struct Collider{
  ColliderType type;
  union ColliderData data;
};


CollisionBehavior get_collision_behavior(EntityType type_A, EntityType type_B);
