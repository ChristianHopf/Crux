#include "distance.h"

DistanceFunction distance_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_PLANE] = min_dist_at_time_AABB_plane,
  [COLLIDER_PLANE][COLLIDER_AABB] = min_dist_at_time_AABB_plane
};

float min_dist_at_time_AABB_plane(struct PhysicsBody *body_AABB, struct PhysicsBody *body_plane, float time){

  // Get pointers to the bodies' colliders
  struct AABB *box = body_AABB->collider;
  struct Plane *plane = body_plane->collider;

  // Get world space AABB to find minimum distance
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_AABB->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
    
  vec3 translationA;
  glm_vec3_copy(body_AABB->position, translationA);
  glm_vec3_muladds(body_AABB->velocity, time, translationA);
    
  struct AABB worldAABB_A = {0};
  AABB_update(&box, rotationA, translationA, &worldAABB_A);

  // Get radius of the extents' projection interval onto the plane's normal
  float r =
    worldAABB_A.extents[0] * fabs(plane->normal[0]) +
    worldAABB_A.extents[1] * fabs(plane->normal[1]) +
    worldAABB_A.extents[2] * fabs(plane->normal[2]); 
  // Get distance from the center of AABB to the plane
  float s = glm_dot(plane->normal, worldAABB_A.center) - plane->distance;

  // If distance (s - r) is negative, their minimum distance is 0
  return glm_max(s - r, 0);
}
