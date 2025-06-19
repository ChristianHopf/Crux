#include "physics/narrow_phase.h"

#define EPSILON 0.0001

NarrowPhaseFunction narrow_phase_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_PLANE] = narrow_phase_AABB_plane,
  // [COLLIDER_PLANE][COLLIDER_AABB] = narrow_phase_AABB_plane,
  [COLLIDER_SPHERE][COLLIDER_PLANE] = narrow_phase_sphere_plane,
  // [COLLIDER_PLANE][COLLIDER_SPHERE] = narrow_phase_sphere_plane,
};


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
  printf("NARROW PHASE WORLD SPACE AABB\n");
  print_aabb(&worldAABB_A);

  // Get relative velocity
  vec3 rel_v;
  glm_vec3_copy(body_AABB->velocity, rel_v);

  // Get radius of projection interval
  float r =
    worldAABB_A.extents[0] * fabs(plane->normal[0]) +
    worldAABB_A.extents[1] * fabs(plane->normal[1]) +
    worldAABB_A.extents[2] * fabs(plane->normal[2]); 

  // Get distance from center of AABB to plane
  // printf("Dot product of plane normal and world aabb center %f\n", glm_dot(plane->normal, worldAABB_A.center));
  // printf("Plane distance from origin: %f\n", plane->distance);
  float s = glm_dot(plane->normal, worldAABB_A.center) - plane->distance;
  // printf("Distance from center to plane: %f\n", s);

  // Get dot product of normal and relative velocity
  // - n*v = 0 => moving parallel
  // - n*v < 0 => moving towards plane
  // - n*v > 0 => moving away from the plane
  float n_dot_v = glm_dot(plane->normal, rel_v);
  // printf("r: %f, s: %f, n_dot_v: %f\n", r, s, n_dot_v);

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
      printf("Moving toward plane, result hit time (towards plane)%f\n", result.hit_time);
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
    // if (result.colliding){
    //   glm_vec3_copy(worldAABB_A.center, result.point_of_contact);
    //   glm_vec3_muladds(body_AABB->velocity, result.hit_time, result.point_of_contact);
    //
    //   if (fabs(s) < r){
    //     float dist = glm_dot(plane->normal, result.point_of_contact) - plane->distance;
    //     result.penetration = dist;
    //     // printf("Result penetration %f\n", s - dist);
    //     glm_vec3_mulsubs(plane->normal, dist, result.point_of_contact);
    //   }
    // }
    // glm_vec3_copy(worldAABB_A.center, result.point_of_contact);
    // glm_vec3_muladds(body_AABB->velocity, result.hit_time, result.point_of_contact);
    // glm_vec3_mulsubs(plane->normal, r, result.point_of_contact);
  }

  return result;
}

struct CollisionResult narrow_phase_sphere_sphere(struct PhysicsBody *body_sphere_A, struct PhysicsBody *body_sphere_B, float delta_time){
  struct Sphere *sphere_A = &body_sphere_A->collider.data.sphere;
  struct Sphere *sphere_B = &body_sphere_B->collider.data.sphere;
  struct CollisionResult result = {0};

  // Get world space spheres
  struct Sphere world_sphere_A = {0};
  glm_vec3_add(sphere_A->center, body_sphere_A->position, world_sphere_A.center);
  world_sphere_A.radius = sphere_A->radius * body_sphere_A->scale[0];

  struct Sphere world_sphere_B = {0};
  glm_vec3_add(sphere_B->center, body_sphere_B->position, world_sphere_B.center);
  world_sphere_B.radius = sphere_B->radius * body_sphere_B->scale[0];

  // s - difference between centers
  // v - relative velocity
  // r - sum of radii
  vec3 s, v;
  float r;
  glm_vec3_sub(world_sphere_A.center, world_sphere_B.center, s);
  glm_vec3_sub(body_sphere_A->velocity, body_sphere_B->velocity, v);
  r = world_sphere_A.radius + world_sphere_B.radius;

  // If the distance between spheres is less than the sum of their radii,
  // they're already colliding
  float c = glm_dot(s, s) - (r * r);
  if (c < 0.0f){
    result.hit_time = 0;
    result.colliding = true;
  }
  else{
    // If the magnitude of the relative velocity is 0,
    // the spheres aren't moving relative to each other
    float a = glm_dot(v, v);
    if (a < EPSILON){
      result.hit_time = -1;
      result.colliding = false;
    }

    // If the projection of the relative velocity on the difference vector is positive,
    // the spheres are moving away from each other.
    float b = glm_dot(v, s);
    if (b >= 0.0f){
      result.hit_time = -1;
      result.colliding = false;
    }

    // If there is no real-valued root for the discriminant of the quadratic equation for t,
    // the spheres do not collide
    float d = (b * b) - a * c;
    if (d < 0.0f){
      result.hit_time = -1;
      result.colliding = false;
    }

    // Two solutions for t, we want the first (smaller) one
    result.hit_time = (-b - sqrt(d)) / a;
    result.colliding = true;
  }
  
  return result;
}

struct CollisionResult narrow_phase_sphere_plane(struct PhysicsBody *body_sphere, struct PhysicsBody *body_plane, float delta_time){
  struct Sphere *sphere = &body_sphere->collider.data.sphere;
  struct Plane *plane = &body_plane->collider.data.plane;
  struct CollisionResult result = {0};

  // Get world space sphere
  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_sphere->position, world_sphere.center);
  // glm_vec3_muladds(body_A->velocity, time, world_sphere.center);
  // glm_vec3_scale(world_sphere.center, body_sphere->scale[0], world_sphere.center);
  world_sphere.radius = sphere->radius * body_sphere->scale[0];

  // Get signed distance from sphere center to plane
  float s = glm_dot(world_sphere.center, plane->normal) - plane->distance;
  printf("Signed distance from sphere center to plane: %f\n", s);

  // Get velocity along normal
  float n_dot_v = glm_dot(body_sphere->velocity, plane->normal);

  // Compute product of signed distance and normal velocity
  float discriminant = s * n_dot_v;

  // Solve for t based on plane displacement according to the sign of:
  // (n * V)(n * C - d)
  // - term > 0 => sphere is moving away from the plane
  // - term < 0 => sphere is moving towards the plane
  // - term == 0 => sphere is moving parallel to the plane
  if (discriminant == 0){
    if (fabs(s) <= world_sphere.radius){
      result.hit_time = 0;
      result.colliding = true;
    }
    else{
      result.hit_time = -1;
      result.colliding = false;
    }
  }
  else{
    if (discriminant < 0){
      // t = (r - ((n * C) - d)) / (n * v)
      result.hit_time = (world_sphere.radius - s) / n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }
    else{
      // t = (-r - ((n * C) - d)) / (n * v)
      result.hit_time = (-world_sphere.radius - s) / n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }

    // If hit_time is outside of interval, check if already colliding
    if (!result.colliding){
      if (abs(s <= world_sphere.radius)){
        result.hit_time = 0;
        result.colliding = true;
      }
      else{
        result.hit_time = -1;
      }
    }
  }

  return result;
}
