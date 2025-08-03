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
  // Get pointers to the bodies' colliders
  struct AABB *box = &body_A->collider.data.aabb;
  struct Capsule *capsule = &body_B->collider.data.capsule;

  struct AABB world_AABB = {0};
  struct AABB world_AABB_final = {0};
  struct Capsule world_capsule = {0};

  // Apply world transform to bodies
  if (body_A->scene_node){
    vec3 world_position, world_rotation, world_scale;
    glm_mat4_mulv3(body_A->scene_node->parent_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position);
    glm_decompose_scalev(body_A->scene_node->parent_node->world_transform, world_scale);
    mat3 rotation_mat3;
    glm_mat4_pick3(body_A->scene_node->parent_node->world_transform, rotation_mat3);
    if (world_scale[0] != 0.0f){
      glm_mat3_scale(rotation_mat3, 1.0f / world_scale[0]);
    }

    vec3 euler_radians, euler_degrees;
    
    // Assuming rotation_mat is normalized (scale removed)
    // XYZ order: Rx * Ry * Rz
    // rotation_mat = [r11 r12 r13]
    //               [r21 r22 r23]
    //               [r31 r32 r33]
    float r11 = rotation_mat3[0][0];
    float r12 = rotation_mat3[0][1];
    float r13 = rotation_mat3[0][2];
    float r21 = rotation_mat3[1][0];
    float r31 = rotation_mat3[2][0];
    float r32 = rotation_mat3[2][1];
    float r33 = rotation_mat3[2][2];

    // Pitch (Y) = asin(-r31)
    euler_radians[1] = asinf(-r31); // -pi/2 to pi/2

    // Handle gimbal lock cases (r31 ≈ ±1)
    if (fabsf(r31) > 0.9999f) {
      // Gimbal lock: pitch is ±90 degrees
      euler_radians[0] = 0.0f; // Roll (X) is undefined, set to 0
      euler_radians[2] = atan2f(r12, r11); // Yaw (Z)
    } else {
      // Roll (X) = atan2(r32, r33)
      euler_radians[0] = -atan2f(r32, r33);
      // Yaw (Z) = atan2(r21, r11)
      euler_radians[2] = -atan2f(r21, r11);
    }

    // Convert to degrees
    euler_degrees[0] = glm_deg(euler_radians[0]); // Roll (X)
    euler_degrees[1] = glm_deg(euler_radians[1]); // Pitch (Y)
    euler_degrees[2] = glm_deg(euler_radians[2]); // Yaw (Z)
    AABB_update(box, rotation_mat3, world_position, world_scale, &world_AABB);
  }
  else{
    world_AABB = *box;
  }
  if (body_B->scene_node){
    // Transform capsule
    glm_mat4_mulv3(body_B->scene_node->world_transform, capsule->segment_A, 1.0f, world_capsule.segment_A);
    glm_mat4_mulv3(body_B->scene_node->world_transform, capsule->segment_B, 1.0f, world_capsule.segment_B);
  }

  // AABB local transform update
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  vec3 translationA, scaleA;
  glm_vec3_copy(body_A->position, translationA);
  glm_vec3_muladds(body_A->velocity, time, translationA);
  glm_vec3_copy(body_A->scale, scaleA);
  AABB_update(&world_AABB, rotationA, translationA, scaleA, &world_AABB_final);

  // Get world space capsule
  glm_vec3_scale(capsule->segment_A, body_B->scale[0], world_capsule.segment_A);
  glm_vec3_scale(capsule->segment_B, body_B->scale[0], world_capsule.segment_B);
  vec3 rotatedA, rotatedB;
  glm_euler_xyz(body_B->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  glm_mat3_mulv(rotationA, world_capsule.segment_A, world_capsule.segment_A);
  glm_mat3_mulv(rotationA, world_capsule.segment_B, world_capsule.segment_B);
  glm_vec3_add(world_capsule.segment_A, body_B->position, world_capsule.segment_A);
  glm_vec3_add(world_capsule.segment_B, body_B->position, world_capsule.segment_B);
  world_capsule.radius = capsule->radius * body_A->scale[0];

  // Find closest point on segment to AABB center
  vec3 segment, A_to_center;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  glm_vec3_sub(world_AABB_final.center, world_capsule.segment_A, A_to_center);
  float proj = glm_dot(segment, A_to_center);
  // Normalize projection of A->center onto segment, clamp between 0 and 1
  float t = proj / glm_dot(segment, segment);
  t = glm_clamp(t, 0.0f, 1.0f);

  // Closest point on the segment is segment_A + segment vector scaled by t
  vec3 closest_point;
  glm_vec3_copy(world_capsule.segment_A, closest_point);
  glm_vec3_muladds(segment, t, world_capsule.segment_A);

  // Closest point on the AABB is the closest point on the segment clamped
  // to the extents of the AABB
  vec3 q;
  for (int i = 0; i < 3; i++){
    q[i] = glm_clamp(closest_point[i], world_AABB_final.center[i] - world_AABB_final.extents[i], world_AABB_final.center[i] + world_AABB_final.extents[i]);
  }

  vec3 pq;
  glm_vec3_sub(q, closest_point, pq);
  float distance = glm_vec3_norm(pq);
  printf("Min dist between AABB and capsule is %f\n", glm_max(distance - world_capsule.radius, 0));
  return glm_max(distance - world_capsule.radius, 0);

  // Find AABB vertex with minimum distance (point to line segment distance)
  // float min_dist = FLT_MAX;
  // vec3 segment;
  // glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  // print_glm_vec3(world_capsule.segment_A, "World capsule segment A");
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] - world_AABB_final.extents[0], world_AABB_final.center[1] - world_AABB_final.extents[1], world_AABB_final.center[2] - world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] - world_AABB_final.extents[0], world_AABB_final.center[1] - world_AABB_final.extents[1], world_AABB_final.center[2] + world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] - world_AABB_final.extents[0], world_AABB_final.center[1] + world_AABB_final.extents[1], world_AABB_final.center[2] - world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] - world_AABB_final.extents[0], world_AABB_final.center[1] + world_AABB_final.extents[1], world_AABB_final.center[2] + world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] + world_AABB_final.extents[0], world_AABB_final.center[1] - world_AABB_final.extents[1], world_AABB_final.center[2] - world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] + world_AABB_final.extents[0], world_AABB_final.center[1] - world_AABB_final.extents[1], world_AABB_final.center[2] + world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] + world_AABB_final.extents[0], world_AABB_final.center[1] + world_AABB_final.extents[1], world_AABB_final.center[2] - world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // min_dist = fminf(distance_point_to_segment((vec3){world_AABB_final.center[0] + world_AABB_final.extents[0], world_AABB_final.center[1] + world_AABB_final.extents[1], world_AABB_final.center[2] + world_AABB_final.extents[2]}, world_capsule.segment_A, world_capsule.segment_B), min_dist);
  // printf("Min dist between AABB and capsule is %f\n", min_dist);
  //
  // return glm_max(min_dist - world_capsule.radius, 0);
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
  struct Plane world_plane = {0};

  // Apply world transform to bodies
  // TODO Give Player a SceneNode
  if (body_A->scene_node){
    // Transform capsule
    glm_mat4_mulv3(body_A->scene_node->world_transform, capsule->segment_A, 1.0f, world_capsule.segment_A);
    glm_mat4_mulv3(body_A->scene_node->world_transform, capsule->segment_B, 1.0f, world_capsule.segment_B);
  }
  if (body_B->scene_node){
    mat3 rotation_mat3;
    vec3 world_position, world_rotation, world_scale;
    glm_mat4_mulv3(body_B->scene_node->parent_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position);
    glm_decompose_scalev(body_B->scene_node->parent_node->world_transform, world_scale);
    glm_mat4_pick3(body_B->scene_node->parent_node->world_transform, rotation_mat3);
    if (world_scale[0] != 0.0f){
      glm_mat3_scale(rotation_mat3, 1.0f / world_scale[0]);
    }

    // Transform plane
    // print_glm_vec3(plane->normal, "Wall plane normal before transformation");
    // print_glm_mat3(rotation_mat3, "Rotation mat3");
    // glm_vec3_copy(plane->normal, world_plane.normal);
    glm_mat3_mulv(rotation_mat3, plane->normal, world_plane.normal);
    glm_vec3_normalize(world_plane.normal);
    world_plane.distance = (plane->distance) + glm_vec3_dot(world_position, world_plane.normal);
    // if (plane->distance == -5){
    //   print_glm_vec3(plane->normal, "Wall plane normal");
    //   print_glm_mat4(body_B->scene_node->world_transform, "World transform");
    //   print_glm_vec3(world_plane.normal, "World wall plane normal");
    // }
  }

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
  float n_dot_A = glm_dot(world_plane.normal, world_capsule.segment_A);
  vec3 segment;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  float n_dot_segment = glm_dot(world_plane.normal, segment);

  vec3 closest_point;
  float t = (world_plane.distance - n_dot_A) / n_dot_segment;
  if (t <= 0) glm_vec3_copy(world_capsule.segment_A, closest_point);
  else if (t >= 1) glm_vec3_copy(world_capsule.segment_B, closest_point);
  else glm_vec3_lerp(world_capsule.segment_A, world_capsule.segment_B, t, closest_point);
  // print_glm_vec3(closest_point, "Closest point");

  // Distance from closest point to plane
  float s = glm_dot(closest_point, world_plane.normal) - world_plane.distance;
  float distance = fabs(s) - world_capsule.radius;
  // if (!body_A->scene_node){
  //   printf("s: %f, distance: %f\n", s, distance);
  // }

  return glm_max(distance, 0);
}
