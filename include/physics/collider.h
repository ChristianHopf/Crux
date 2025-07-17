#pragma once

#include <cglm/cglm.h>
#include "aabb.h"
#include "sphere.h"
#include "capsule.h"
#include "plane.h"

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
