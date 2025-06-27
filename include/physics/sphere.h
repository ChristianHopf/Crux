#pragma once

#include <cglm/cglm.h>

struct Sphere {
  vec3 center;
  float radius;
};

bool sphere_intersect_aabb();
bool sphere_intersect_sphere(struct Sphere *sphere_A, struct Sphere *sphere_B);
bool sphere_intersect_plane();
