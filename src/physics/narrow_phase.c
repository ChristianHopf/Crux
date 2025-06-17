#include "physics/narrow_phase.h"

struct CollisionResult narrow_phase_AABB_plane(struct PhysicsBody *body_AABB, struct PhysicsBody *body_plane, float delta_time){
  struct AABB *box = &body_AABB->collider.data.aabb;
  struct Plane *plane = &body_plane->collider.data.plane;

  struct CollisionResult result = {0};

  // Get a world space version of the current AABB
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_AABB->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  vec3 translationA;
  glm_vec3_copy(body_AABB->position, translationA);
  vec3 scaleA;
  glm_vec3_copy(body_AABB->scale, scaleA);
  struct AABB worldAABB_A = {0};
  AABB_update(box, rotationA, translationA, scaleA, &worldAABB_A);

  // Get relative velocity
  vec3 rel_v;
  glm_vec3_copy(body_AABB->velocity, rel_v);

  // Get radius of projection interval
  float r =
    worldAABB_A.extents[0] * fabs(plane->normal[0]) +
    worldAABB_A.extents[1] * fabs(plane->normal[1]) +
    worldAABB_A.extents[2] * fabs(plane->normal[2]); 

  // Get distance from center of AABB to plane
  printf("Dot product of plane normal and world aabb center %f\n", glm_dot(plane->normal, worldAABB_A.center));
  printf("Plane distance from origin: %f\n", plane->distance);
  float s = glm_dot(plane->normal, worldAABB_A.center) - plane->distance;
  // printf("Distance from center to plane: %f\n", s);

  // Get dot product of normal and relative velocity
  // - n*v = 0 => moving parallel
  // - n*v < 0 => moving towards plane
  // - n*v > 0 => moving away from the plane
  float n_dot_v = glm_dot(plane->normal, rel_v);
  printf("r: %f, s: %f, n_dot_v: %f\n", r, s, n_dot_v);

  // n*v == 0 => parallel movement
  if (n_dot_v == 0){
    if (fabs(s) <= r){
      result.colliding = true;
      result.hit_time = 0;
    }
    else{
      result.colliding = false;
      result.hit_time = -1;
    }
  }
  // If n*v != 0, solve for t.
  // Ericson's equation:
  // t = (r + d - (n * C)) / (n * v)
  // Is equivalent to:
  // t = (r - ((n * C) - d)) / (n * v), or
  // t = (r - s) / (n * v)
  else {
    // Moving towards plane
    if (n_dot_v < 0){
      result.hit_time = (r - s) / -n_dot_v;
      printf("Result hit time (towards plane)%f\n", result.hit_time);
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }
    // Moving away from plane
    else{
      // Already intersecting
      if (fabs(s) <= r){
        result.hit_time = 0;
        result.colliding = true;
      } else {
        result.hit_time = (r + s) / -n_dot_v;
        result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
        // printf("Moving away result hit time %f\n", result.hit_time);
      }
    }

    // Sometimes hit_time is bigger than delta_time. Clamp to delta_time
    if (result.colliding){
      result.hit_time = fmaxf(0.0f, fminf(result.hit_time, delta_time));
    }

    // If hit_time is outside of time interval,
    // no collision unless s is within r
    if (!result.colliding){
      if (fabs(s) <= r){
        result.hit_time = 0;
        result.colliding = true;
      }
      else{
        result.hit_time = -1;
      }
    } 

    // Point of contact Q = C(t) - rn
    // vec3 Q;
    if (result.colliding){
      glm_vec3_copy(worldAABB_A.center, result.point_of_contact);
      glm_vec3_muladds(body_AABB->velocity, result.hit_time, result.point_of_contact);

      if (fabs(s) < r){
        float dist = glm_dot(plane->normal, result.point_of_contact) - plane->distance;
        result.penetration = dist;
        // printf("Result penetration %f\n", s - dist);
        glm_vec3_mulsubs(plane->normal, dist, result.point_of_contact);
      }
    }
    // glm_vec3_copy(worldAABB_A.center, result.point_of_contact);
    // glm_vec3_muladds(body_AABB->velocity, result.hit_time, result.point_of_contact);
    // glm_vec3_mulsubs(plane->normal, r, result.point_of_contact);
  }

  return result;
}
