#include "distance.h"

DistanceFunction distance_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_PLANE] = min_dist_at_time_AABB_plane,
  // [COLLIDER_PLANE][COLLIDER_AABB] = min_dist_at_time_AABB_plane,
  [COLLIDER_SPHERE][COLLIDER_PLANE] = min_dist_at_time_sphere_plane,
  // [COLLIDER_PLANE][COLLIDER_SPHERE] = min_dist_at_time_sphere_plane,
};

float min_dist_at_time_AABB_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){

  // Get pointers to the bodies' colliders
  struct AABB *box = &body_A->collider.data.aabb;
  struct Plane *plane = &body_B->collider.data.plane;

  // Get world space AABB to find minimum distance
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  vec3 translationA;
  glm_vec3_copy(body_A->position, translationA);
  glm_vec3_muladds(body_A->velocity, time, translationA);
  vec3 scaleA;
  glm_vec3_copy(body_A->scale, scaleA);
    
  struct AABB worldAABB_A = {0};
  AABB_update(box, rotationA, translationA, scaleA, &worldAABB_A);

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

float min_dist_at_time_sphere_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  // Get pointers to the bodies' colliders
  struct Sphere *sphere = &body_A->collider.data.sphere;
  struct Plane *plane = &body_B->collider.data.plane;

  struct Sphere world_sphere;
  glm_vec3_copy(body_A->position, world_sphere.center);
  glm_vec3_muladds(body_A->velocity, time, world_sphere.center);
  glm_vec3_scale(world_sphere.center, body_A->scale[0], world_sphere.center);
  glm_vec3_scale(world_sphere.radius, body_A->scale[0], world_sphere.radius);

  // Not finding a radius of projection, just want signed distance
  float s = glm_dot(world_sphere.center, plane->normal) - plane->distance;
  float distance = fabs(s) - world_sphere->radius;

  return glm_max(distance, 0);
}
