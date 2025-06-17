#pragma once

#include <cglm/cglm.h>

struct Sphere {
  vec3 center;
  float radius;
};

bool sphere_intersect_aabb();
bool sphere_intersect_sphere();
bool sphere_intersect_plane();
