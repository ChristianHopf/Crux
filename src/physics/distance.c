#include "distance.h"
#include "physics/utils.h"

DistanceFunction distance_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_AABB] = min_dist_at_time_AABB_AABB,
  [COLLIDER_AABB][COLLIDER_SPHERE] = min_dist_at_time_AABB_sphere,
  [COLLIDER_AABB][COLLIDER_CAPSULE] = min_dist_at_time_AABB_capsule,
  [COLLIDER_AABB][COLLIDER_PLANE] = min_dist_at_time_AABB_plane,
  [COLLIDER_SPHERE][COLLIDER_SPHERE] = min_dist_at_time_sphere_sphere,
  [COLLIDER_SPHERE][COLLIDER_CAPSULE] = min_dist_at_time_sphere_capsule,
  [COLLIDER_SPHERE][COLLIDER_PLANE] = min_dist_at_time_sphere_plane,
  [COLLIDER_CAPSULE][COLLIDER_CAPSULE] = min_dist_at_time_capsule_capsule,
  [COLLIDER_CAPSULE][COLLIDER_PLANE] = min_dist_at_time_capsule_plane
};


float min_dist_at_time_AABB_AABB(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  struct AABB *aabb_A = &body_A->collider.data.aabb;
  struct AABB *aabb_B = &body_B->collider.data.aabb;

  // Get world space bodies
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  vec3 translationA;
  glm_vec3_copy(body_A->position, translationA);
  vec3 scaleA;
  glm_vec3_copy(body_A->scale, scaleA);
  struct AABB world_AABB_A = {0};
  AABB_update(aabb_A, rotationA, translationA, scaleA, &world_AABB_A);

  // Get world space bodies
  mat4 eulerB;
  mat3 rotationB;
  glm_euler_xyz(body_B->rotation, eulerB);
  glm_mat4_pick3(eulerB, rotationB);
  vec3 translationB;
  glm_vec3_copy(body_B->position, translationB);
  vec3 scaleB;
  glm_vec3_copy(body_B->scale, scaleB);
  struct AABB world_AABB_B = {0};
  AABB_update(aabb_B, rotationB, translationB, scaleB, &world_AABB_B);

  // Find squared distance on all 3 axes
  float distance_squared = 0.0f;
  vec3 min_A, max_A;
  vec3 min_B, max_B;
  glm_vec3_sub(world_AABB_A.center, world_AABB_A.extents, min_A);
  glm_vec3_add(world_AABB_A.center, world_AABB_A.extents, max_A);
  glm_vec3_sub(world_AABB_B.center, world_AABB_B.extents, min_B);
  glm_vec3_add(world_AABB_B.center, world_AABB_B.extents, max_B);
  for(int i = 0; i < 3; i++){
    if (min_B[i] > max_A[i]){
      distance_squared += (min_B[i] - max_A[i]) * (min_B[i] - max_A[i]);
    }
    else if (max_B[i] < min_A[i]){
      distance_squared += (min_A[i] - max_B[i]) * (min_A[i] - max_B[i]);
    }
  }

  return sqrt(distance_squared);
}

float min_dist_at_time_AABB_sphere(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  struct AABB *box = &body_A->collider.data.aabb;
  struct Sphere *sphere = &body_B->collider.data.sphere;

  // Get world space bodies
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);

  vec3 translationA, scaleA;
  glm_vec3_copy(body_A->position, translationA);
  glm_vec3_muladds(body_A->velocity, time, translationA);
  glm_vec3_copy(body_A->scale, scaleA);
    
  struct AABB world_AABB = {0};
  AABB_update(box, rotationA, translationA, scaleA, &world_AABB);

  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_B->position, world_sphere.center);
  glm_vec3_muladds(body_B->velocity, time, world_sphere.center);
  world_sphere.radius = sphere->radius * body_B->scale[0];

  // Get squared distance between center and AABB
  float distance_squared = 0.0f;
  for(int i = 0; i < 3; i++){
    float v = world_sphere.center[i];
    float min_extent = world_AABB.center[i] - world_AABB.extents[i];
    float max_extent = world_AABB.center[i] + world_AABB.extents[i];

    if (v < min_extent) distance_squared += (min_extent - v) * (min_extent - v);
    if (v > max_extent) distance_squared += (v - max_extent) * (v - max_extent);
  }

  // Return 0 or sqrt like with sphere-sphere
  return distance_squared < (world_sphere.radius * world_sphere.radius) ? 0.0f : sqrt(distance_squared) - world_sphere.radius;
}

float min_dist_at_time_AABB_capsule(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){

}

float min_dist_at_time_AABB_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){

  // Get pointers to the bodies' colliders
  struct AABB *box = &body_A->collider.data.aabb;
  struct Plane *plane = &body_B->collider.data.plane;

  // Get world space AABB to find minimum distance
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);

  vec3 translationA, scaleA;
  glm_vec3_copy(body_A->position, translationA);
  glm_vec3_muladds(body_A->velocity, time, translationA);
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

float min_dist_at_time_sphere_sphere(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  struct Sphere *sphere_A = &body_A->collider.data.sphere;
  struct Sphere *sphere_B = &body_B->collider.data.sphere;

  struct Sphere world_sphere_A = {0};
  glm_vec3_add(sphere_A->center, body_A->position, world_sphere_A.center);
  glm_vec3_muladds(body_A->velocity, time, world_sphere_A.center);
  world_sphere_A.radius = sphere_A->radius * body_A->scale[0];

  struct Sphere world_sphere_B = {0};
  glm_vec3_add(sphere_B->center, body_B->position, world_sphere_B.center);
  glm_vec3_muladds(body_B->velocity, time, world_sphere_B.center);
  world_sphere_B.radius = sphere_B->radius * body_B->scale[0];

  // Distance between spheres: distance between centers - sum of radii
  vec3 difference;
  glm_vec3_sub(world_sphere_A.center, world_sphere_B.center, difference);
  float distance_squared = glm_dot(difference, difference);
  float radius_sum = world_sphere_A.radius + world_sphere_B.radius;

  return distance_squared < (radius_sum * radius_sum) ? 0.0f : sqrt(distance_squared) - radius_sum;
}

float min_dist_at_time_sphere_capsule(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){

}

float min_dist_at_time_sphere_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  // Get pointers to the bodies' colliders
  struct Sphere *sphere = &body_A->collider.data.sphere;
  struct Plane *plane = &body_B->collider.data.plane;

  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_A->position, world_sphere.center);
  glm_vec3_muladds(body_A->velocity, time, world_sphere.center);
  // glm_vec3_scale(world_sphere.center, body_A->scale[0], world_sphere.center);
  world_sphere.radius = sphere->radius * body_A->scale[0];

  // Not finding a radius of projection, just want signed distance
  float s = glm_dot(world_sphere.center, plane->normal) - plane->distance;
  float distance = fabs(s) - world_sphere.radius;

  return glm_max(distance, 0);
}

float min_dist_at_time_capsule_capsule(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){

}

float min_dist_at_time_capsule_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time){
  // Get pointers to the bodies' colliders
  struct Capsule *capsule = &body_A->collider.data.capsule;
  struct Plane *plane = &body_B->collider.data.plane;

  struct Capsule world_capsule = {0};
  // Scale
  glm_vec3_scale(capsule->segment_A, body_A->scale[0], world_capsule.segment_A);
  glm_vec3_scale(capsule->segment_B, body_A->scale[0], world_capsule.segment_B);
  // Rotate
  mat4 eulerA;
  mat3 rotationA;
  vec3 rotatedA, rotatedB;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  glm_mat3_mulv(rotationA, world_capsule.segment_A, world_capsule.segment_A);
  glm_mat3_mulv(rotationA, world_capsule.segment_B, world_capsule.segment_B);

  glm_vec3_add(world_capsule.segment_A, body_A->position, world_capsule.segment_A);
  glm_vec3_add(world_capsule.segment_B, body_A->position, world_capsule.segment_B);
  world_capsule.radius = capsule->radius * body_A->scale[0];
  // printf("ORIGINAL CAPSULE\n");
  // print_glm_vec3(capsule->segment_A, "Segment A");
  // print_glm_vec3(capsule->segment_B, "Segment B");
  // printf("Radius: %f\n", capsule->radius);
  //
  // printf("WORLD CAPSULE\n");
  // print_glm_vec3(world_capsule.segment_A, "Segment A");
  // print_glm_vec3(world_capsule.segment_B, "Segment B");
  // printf("Radius: %f\n", world_capsule.radius);

  // Get signed distance from capsule to plane: distance from point to plane minus radius
  // - A capsule is a sphere-swept volume, which is just a line segment and a radius
  // - The closest point on the segment to the plane is the closest point on the line to the plane,
  //   clamped to the segment
  // - Line segment: P(t) = A + t(B - A), where 0 <= t <= 1
  // - Plane: n * X = d
  // - Solve n * P(t) = d for t: t = (d - (n * A)) / (n * (B - A))
  //   If t <= 0, the closest point is A
  //   If t >= 1, the closest point is B
  //   Else, compute P(t) for a point on the segment
  float n_dot_A = glm_dot(plane->normal, world_capsule.segment_A);
  vec3 segment;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  float n_dot_segment = glm_dot(plane->normal, segment);

  vec3 closest_point;
  float t = (plane->distance - n_dot_A) / n_dot_segment;
  if (t <= 0) glm_vec3_copy(world_capsule.segment_A, closest_point);
  else if (t >= 1) glm_vec3_copy(world_capsule.segment_B, closest_point);
  else glm_vec3_lerp(world_capsule.segment_A, world_capsule.segment_B, t, closest_point);
  // print_glm_vec3(closest_point, "Closest point");

  // Distance from closest point to plane
  float s = glm_dot(closest_point, plane->normal) - plane->distance;
  float distance = fabs(s) - world_capsule.radius;
  // printf("s: %f, distance: %f\n", s, distance);

  return glm_max(distance, 0);
}
