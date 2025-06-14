#pragma once

#include <cglm/cglm.h>
#include "aabb.h"
#include "plane.h"

typedef enum {
  COLLIDER_AABB,
  COLLIDER_PLANE,
} ColliderType;

struct Collider {
  COLLIDER_TYPE type;
  // Thanks Low Level
  union {
    struct AABB aabb;
    struct Plane plane;
  } data;
  vec3 position;
  // Spheres wouldn't care about rotation... but whatever
  vec3 rotation;
  vec3 velocity;
};

// struct PlaneCollider {
//   vec3 normal;
//   float distance;
// };
