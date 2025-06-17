#include "sphere.h"
#include <cglm/vec3.h>

void sphere_intersect_sphere(struct Sphere *sphere_A, struct Sphere *sphere_B){
  vec3 distance;
  glm_vec3_sub(sphere_A->center, sphere_B->center, distance);
  float dist2 = glm_dot(distance, distance);

  float radius_sum = sphere_A->radius + sphere_B->radius;
  return dist2 <= radiusSum * radiusSum;
}
