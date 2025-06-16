#pragma once

#include <cglm/cglm.h>
#include "aabb.h"
#include "plane.h"

typedef enum {
  COLLIDER_AABB = 0,
  COLLIDER_PLANE,
} ColliderType;

union ColliderData {
  struct AABB aabb;
  struct Plane plane;
};

struct Collider{
  ColliderType type;
  union ColliderData data;
};
