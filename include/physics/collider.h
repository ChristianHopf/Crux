#pragma once

#include <cglm/cglm.h>
#include "aabb.h"
#include "plane.h"
#include "sphere.h"

typedef enum {
  COLLIDER_AABB = 0,
  COLLIDER_PLANE,
  COLLIDER_SPHERE,
  COLLIDER_COUNT
} ColliderType;

union ColliderData {
  struct AABB aabb;
  struct Plane plane;
  struct Sphere sphere;
};

struct Collider{
  ColliderType type;
  union ColliderData data;
};
