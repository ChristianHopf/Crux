#pragma once

#include <cglm/cglm.h>
#include <stdbool.h>
#include "physics/world.h"

struct CollisionResult {
  float hit_time;
  vec3 point_of_contact;
  bool colliding;
};

struct CollisionResult narrow_phase_AABB_plane(struct PhysicsBody *body_AABB, struct PhysicsBody *body_plane, float delta_time);
