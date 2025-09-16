#include "entity.h"
#include "audio_manager.h"
#include "scene.h"
#include "resolution.h"
#include "collider.h"
#include "utils.h"

ResolutionFunction resolution_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_AABB] = resolve_collision_AABB_AABB,
  [COLLIDER_AABB][COLLIDER_SPHERE] = resolve_collision_AABB_sphere,
  [COLLIDER_AABB][COLLIDER_CAPSULE] = resolve_collision_AABB_capsule,
  [COLLIDER_AABB][COLLIDER_PLANE] = resolve_collision_AABB_plane,
  [COLLIDER_SPHERE][COLLIDER_SPHERE] = resolve_collision_sphere_sphere,
  [COLLIDER_SPHERE][COLLIDER_CAPSULE] = resolve_collision_sphere_capsule,
  [COLLIDER_SPHERE][COLLIDER_PLANE] = resolve_collision_sphere_plane,
  [COLLIDER_CAPSULE][COLLIDER_CAPSULE] = resolve_collision_capsule_capsule,
  [COLLIDER_CAPSULE][COLLIDER_PLANE] = resolve_collision_capsule_plane
};


void resolve_collision_AABB_AABB(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  struct AABB *aabb_A = &body_A->collider.data.aabb;
  struct AABB *aabb_B = &body_B->collider.data.aabb;
  struct AABB world_AABB_A = {0};
  struct AABB world_AABB_B = {0};

  // Move bodies according to velocity
  float gravity = 9.8f;
  vec3 velocity_before_A, velocity_before_B;
  if (body_A->dynamic){
    glm_vec3_copy(body_A->velocity, velocity_before_A);
    velocity_before_A[1] -= gravity * result.hit_time;
    glm_vec3_muladds(velocity_before_A, result.hit_time, body_A->position);
  }
  if (body_B->dynamic){
    glm_vec3_copy(body_B->velocity, velocity_before_B);
    velocity_before_B[1] -= gravity * result.hit_time;
    glm_vec3_muladds(velocity_before_B, result.hit_time, body_B->position);
  }
  // glm_vec3_copy(body_B->velocity, velocity_before_B);
  // velocity_before_B[1] -= gravity * result.hit_time;
  // glm_vec3_muladds(velocity_before_B, result.hit_time, body_B->position);

  if (body_A->scene_node){
    vec3 world_position_A, world_rotation_A, world_scale_A;
    glm_mat4_mulv3(body_A->scene_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position_A);
    glm_decompose_scalev(body_A->scene_node->world_transform, world_scale_A);
    mat3 rotation_mat3_A;
    glm_mat4_pick3(body_A->scene_node->world_transform, rotation_mat3_A);
    if (world_scale_A[0] != 0.0f){
      glm_mat3_scale(rotation_mat3_A, 1.0f / world_scale_A[0]);
    }
    glm_vec3_muladds(velocity_before_A, result.hit_time, world_position_A);
    AABB_update(aabb_A, rotation_mat3_A, world_position_A, world_scale_A, &world_AABB_A);
  }
  if (body_B->scene_node){
    vec3 world_position_B, world_rotation_B, world_scale_B;
    glm_mat4_mulv3(body_B->scene_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position_B);
    glm_decompose_scalev(body_B->scene_node->world_transform, world_scale_B);
    mat3 rotation_mat3_B;
    glm_mat4_pick3(body_B->scene_node->world_transform, rotation_mat3_B);
    if (world_scale_B[0] != 0.0f){
      glm_mat3_scale(rotation_mat3_B, 1.0f / world_scale_B[0]);
    }
    glm_vec3_muladds(velocity_before_B, result.hit_time, world_position_B);
    AABB_update(aabb_B, rotation_mat3_B, world_position_B, world_scale_B, &world_AABB_B);
  }

  // Find the axis with minimum penetration:
  // - get vector from center A to center B
  // - s = distance between centers on this axis
  // - r = sum of extents on this axis
  // - track the minimum value of r - s on each axis
  // If min_penetration is positive, perform correction for each AABB
  // by the center difference vector, scaled by half the penetration.
  // The contact normal is based on the axis of minimum penetration.
  vec3 center_difference, separation;
  glm_vec3_sub(world_AABB_B.center, world_AABB_A.center, center_difference);
  glm_vec3_zero(separation);
  float min_penetration = FLT_MAX;
  int contact_axis = 0;
  for(int i = 0; i < 3; i++){
    float s = fabs(center_difference[i]);
    float r = world_AABB_A.extents[i] + world_AABB_B.extents[i];
    float penetration = r - s;
    // This won't handle an exact corner collision perfectly realistically
    if (penetration < min_penetration){
      min_penetration = penetration;
      contact_axis = i;
    }
  }

  // If no penetration, no need to correct position or reflect velocities
  if (min_penetration <= 0.0f) return;

  // Get contact normal
  separation[contact_axis] = (center_difference[contact_axis] < 0 ? -1.0f : 1.0f);
  vec3 contact_normal;
  glm_vec3_copy(separation, contact_normal);
  glm_vec3_normalize(contact_normal);

  // Update velocities based on which bodies are dynamic
  // Should collision between static bodies be possible? Probably no reason to support it, at least for now
  if (body_A->dynamic && !body_B->dynamic){
    float v_dot_n = glm_dot(velocity_before_A, contact_normal);
    vec3 reflection;
    glm_vec3_scale(contact_normal, -2.0f * v_dot_n * body_A->restitution, reflection);
    glm_vec3_add(velocity_before_A, reflection, body_A->velocity);
  }
  else if (!body_A->dynamic && body_B->dynamic){
    float v_dot_n = glm_dot(velocity_before_B, contact_normal);
    vec3 reflection;
    glm_vec3_scale(contact_normal, -2.0f * v_dot_n * body_B->restitution, reflection);
    glm_vec3_add(velocity_before_A, reflection, body_B->velocity);
  }
  else{
    vec3 rel_v;
    glm_vec3_sub(velocity_before_B, velocity_before_A, rel_v);
    float separating_v = glm_dot(rel_v, contact_normal);
    if (separating_v < 0.0f){
      glm_vec3_muladds(separation, min_penetration * 0.5f, body_B->position);
      glm_vec3_mulsubs(separation, min_penetration * 0.5f, body_A->position);

      float restitution = 1.0f;
      float v_dot_n_A = glm_dot(velocity_before_A, contact_normal);
      float v_dot_n_B = glm_dot(velocity_before_B, contact_normal);

      // For a perfectly elastic collision,
      // swap the bodies' velocities along the contact normal
      vec3 normal_v_A, normal_v_B;
      glm_vec3_scale(contact_normal, v_dot_n_A, normal_v_A);
      glm_vec3_scale(contact_normal, v_dot_n_B, normal_v_B);

      glm_vec3_sub(velocity_before_A, normal_v_A, body_A->velocity);
      glm_vec3_add(body_A->velocity, normal_v_B, body_A->velocity);

      glm_vec3_sub(velocity_before_B, normal_v_B, body_B->velocity);
      glm_vec3_add(body_B->velocity, normal_v_A, body_B->velocity);
    }
    else{
      // Bodies are moving away from each other, don't change velocities
      glm_vec3_copy(velocity_before_A, body_A->velocity);
      glm_vec3_copy(velocity_before_B, body_B->velocity);
    }
  }
}

void resolve_collision_AABB_sphere(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  struct AABB *box = &body_A->collider.data.aabb;
  struct Sphere *sphere = &body_B->collider.data.sphere;

  // Move bodies according to velocity
  float gravity = 9.8f;
  vec3 velocity_before_A, velocity_before_B;
  glm_vec3_copy(body_A->velocity, velocity_before_A);
  glm_vec3_copy(body_B->velocity, velocity_before_B);
  velocity_before_A[1] -= gravity * result.hit_time;
  velocity_before_B[1] -= gravity * result.hit_time;
  glm_vec3_muladds(velocity_before_A, result.hit_time, body_A->position);
  glm_vec3_muladds(velocity_before_B, result.hit_time, body_B->position);

  // Get world space bodies for penetration correction
  struct AABB world_AABB = {0};
  mat4 eulerA;
  mat3 rotationA;
  vec3 translationA, scaleA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  glm_vec3_copy(body_A->position, translationA);
  glm_vec3_copy(body_A->scale, scaleA);
  AABB_update(box, rotationA, translationA, scaleA, &world_AABB);

  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_B->position, world_sphere.center);
  world_sphere.radius = sphere->radius * body_B->scale[0];

  // Get closest point on AABB to sphere center
  vec3 closest;
  for(int i = 0; i < 3; i++){
    float min = world_AABB.center[i] - world_AABB.extents[i];
    float max = world_AABB.center[i] + world_AABB.extents[i];
    if (min > world_sphere.center[i]){
      closest[i] = min;
    }
    else if (max > world_sphere.center[i]){
      closest[i] = world_sphere.center[i];
    }
    else{
      closest[i] = max;
    }
  }

  vec3 difference, contact_normal;
  glm_vec3_sub(world_sphere.center, closest, difference);
  float s = glm_vec3_norm(difference);
  float penetration = (s < world_sphere.radius) ? (world_sphere.radius - s) + 0.001 : 0.0f;

  if (penetration <= 0) return;

  // Get contact normal
  glm_vec3_copy(difference, contact_normal);
  glm_vec3_normalize(contact_normal);

  // Only correct positions and reflect velocities if the bodies aren't moving away from each other
  vec3 rel_v;
  glm_vec3_sub(velocity_before_B, velocity_before_A, rel_v);
  float separating_v = glm_dot(rel_v, contact_normal);
  if (separating_v < 0.0f){
    glm_vec3_muladds(contact_normal, (penetration / 2), body_B->position);
    glm_vec3_mulsubs(contact_normal, (penetration / 2), body_A->position);

    float restitution = 1.0f;
    float v_dot_n_A = glm_dot(velocity_before_A, contact_normal);
    float v_dot_n_B = glm_dot(velocity_before_B, contact_normal);

    // For a perfectly elastic collision,
    // swap the bodies' velocities along the contact normal
    vec3 normal_v_A, normal_v_B;
    glm_vec3_scale(contact_normal, v_dot_n_A, normal_v_A);
    glm_vec3_scale(contact_normal, v_dot_n_B, normal_v_B);

    glm_vec3_sub(velocity_before_A, normal_v_A, body_A->velocity);
    glm_vec3_add(body_A->velocity, normal_v_B, body_A->velocity);

    glm_vec3_sub(velocity_before_B, normal_v_B, body_B->velocity);
    glm_vec3_add(body_B->velocity, normal_v_A, body_B->velocity);
  }
  else{
    // Bodies are moving away from each other, don't change velocities
    glm_vec3_copy(velocity_before_A, body_A->velocity);
    glm_vec3_copy(velocity_before_B, body_B->velocity);
  }
}

void resolve_collision_AABB_capsule(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  // printf("AABB CAPSULE COLLISION\n");
  struct AABB *box = &body_A->collider.data.aabb;
  struct Capsule *capsule = &body_B->collider.data.capsule;

  struct AABB world_AABB = {0};
  struct AABB world_AABB_final = {0};
  struct Capsule world_capsule = {0};

  // Move bodies according to velocity
  float gravity = 9.8f;
  vec3 velocity_before;
  glm_vec3_copy(body_B->velocity, velocity_before);
  velocity_before[1] -= gravity * result.hit_time;
  glm_vec3_muladds(velocity_before, result.hit_time, body_B->position);

  // Apply world transform to bodies
  if (body_A->scene_node){
    vec3 world_position, world_rotation, world_scale;
    glm_mat4_mulv3(body_A->scene_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position);
    glm_decompose_scalev(body_A->scene_node->world_transform, world_scale);
    mat3 rotation_mat3;
    glm_mat4_pick3(body_A->scene_node->world_transform, rotation_mat3);
    if (world_scale[0] != 0.0f){
      glm_mat3_scale(rotation_mat3, 1.0f / world_scale[0]);
    }
    AABB_update(box, rotation_mat3, world_position, world_scale, &world_AABB_final);
  }
  else{
    world_AABB = *box;
  }
  if (body_B->scene_node){
    // Transform capsule
    glm_mat4_mulv3(body_B->scene_node->world_transform, capsule->segment_A, 1.0f, world_capsule.segment_A);
    glm_mat4_mulv3(body_B->scene_node->world_transform, capsule->segment_B, 1.0f, world_capsule.segment_B);
    glm_vec3_muladds(velocity_before, result.hit_time, world_capsule.segment_A);
    glm_vec3_muladds(velocity_before, result.hit_time, world_capsule.segment_B);
  }
  // Player capsule
  // else{
  //   // Scale
  //   glm_vec3_scale(capsule->segment_A, body_B->scale[0], world_capsule.segment_A);
  //   glm_vec3_scale(capsule->segment_B, body_B->scale[0], world_capsule.segment_B);
  //   // Rotate
  //   mat4 eulerA;
  //   mat3 rotationA;
  //   vec3 rotatedA, rotatedB;
  //   glm_euler_xyz(body_B->rotation, eulerA);
  //   glm_mat4_pick3(eulerA, rotationA);
  //   glm_mat3_mulv(rotationA, world_capsule.segment_A, world_capsule.segment_A);
  //   glm_mat3_mulv(rotationA, world_capsule.segment_B, world_capsule.segment_B);
  //   glm_vec3_add(world_capsule.segment_A, body_B->position, world_capsule.segment_A);
  //   glm_vec3_add(world_capsule.segment_B, body_B->position, world_capsule.segment_B);
  // }
  world_capsule.radius = capsule->radius * body_B->scale[0];

  // Penetration correction
  vec3 segment, A_to_center;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  glm_vec3_sub(world_AABB_final.center, world_capsule.segment_A, A_to_center);
  float proj = glm_dot(segment, A_to_center);
  float t = proj / glm_dot(segment, segment);
  t = glm_clamp(t, 0.0f, 1.0f);

  // Closest point on the segment is segment_A + segment vector scaled by t
  vec3 closest_point;
  glm_vec3_copy(world_capsule.segment_A, closest_point);
  glm_vec3_muladds(segment, t, closest_point);

  // Closest point on the AABB is the segment's closest point clamped to AABB extents
  vec3 q, pq;
  for (int i = 0; i < 3; i++){
    q[i] = glm_clamp(closest_point[i], world_AABB_final.center[i] - world_AABB_final.extents[i], world_AABB_final.center[i] + world_AABB_final.extents[i]);
  }

  glm_vec3_sub(q, closest_point, pq);
  float distance = glm_vec3_norm(pq);

  // Compute discriminant of relative velocity along PQ
  // (vector from closest point on capsule to closest point on AABB)
  glm_vec3_normalize(pq);
  float pq_dot_v = glm_dot(body_B->velocity, pq);
  float penetration = distance < world_capsule.radius ? (world_capsule.radius - distance) + 0.001f : 0.0f;

  if (penetration <= 0.0f) return;

  // TODO Do penetration correction and velocity updates based on whether
  // each body is static or dynamic. For now just assume the AABB is static
  vec3 correction;
  glm_vec3_scale(pq, penetration, correction);
  glm_vec3_sub(body_B->position, correction, body_B->position);

  // Reflect velocity over normal
  float rest_velocity_threshold = 0.1f;
  float v_dot_pq = glm_dot(velocity_before, pq);
  vec3 reflection;
  glm_vec3_scale(pq, -2.0f * v_dot_pq * body_B->restitution, reflection);
  glm_vec3_add(velocity_before, reflection, body_B->velocity);

  // If velocity along the normal is very small,
  // and the normal is opposite gravity, stop (eventually, spheres should be able to roll)
  glm_vec3_negate(pq);
  v_dot_pq = glm_dot(body_B->velocity, pq);
  if (v_dot_pq < 0.5 && glm_dot(pq, (vec3){0.0f, -1.0f, 0.0f}) < 0){
    glm_vec3_zero(body_B->velocity);
    body_B->at_rest = true;
  }
  if (body_B->entity != NULL){
    // entity_play_sound_effect(body_B->entity);
  }
}

void resolve_collision_AABB_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  struct AABB *box = &body_A->collider.data.aabb;
  struct Plane *plane = &body_B->collider.data.plane;

  // First move by velocity according to hit_time, applying gravity until collision
  float gravity = 9.8f;
  vec3 velocity_before;
  glm_vec3_copy(body_A->velocity, velocity_before);
  velocity_before[1] -= gravity * result.hit_time;
  glm_vec3_muladds(velocity_before, result.hit_time, body_A->position);

  struct AABB worldAABB = {0};
  mat4 eulerA;
  mat3 rotationA;
  vec3 translationA, scaleA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  glm_vec3_copy(body_A->position, translationA);
  glm_vec3_copy(body_A->scale, scaleA);
  AABB_update(box, rotationA, translationA, scaleA, &worldAABB);

  vec3 normal;
  glm_vec3_copy(plane->normal, normal);
  float r = worldAABB.extents[0] * fabsf(normal[0]) +
            worldAABB.extents[1] * fabsf(normal[1]) +
            worldAABB.extents[2] * fabsf(normal[2]);
  float s = glm_vec3_dot(normal, worldAABB.center) - plane->distance;
  float penetration = (s < r) ? (r - s) + 0.001 : 0.0f; // Only correct if penetrating
  if (penetration > 0.0f) {
    vec3 correction;
    glm_vec3_scale(normal, penetration, correction);
    glm_vec3_add(body_A->position, correction, body_A->position);
  }

  // Reflect velocity vector over normal
  // float restitution = 1.0f;
  float rest_velocity_threshold = 0.1f;
  float v_dot_n = glm_dot(velocity_before, plane->normal);
  vec3 reflection;
  glm_vec3_scale(plane->normal, -2.0f * v_dot_n * body_A->restitution, reflection);
  glm_vec3_add(velocity_before, reflection, body_A->velocity);

  // If velocity along the normal is very small,
  // and the normal is opposite gravity, stop
  v_dot_n = glm_dot(normal, body_A->velocity);
  if (v_dot_n < 0.5 && glm_dot(normal, (vec3){0.0f, -1.0f, 0.0f}) < 0){
    float distance_to_plane = glm_dot(body_A->position, normal) - plane->distance;
    glm_vec3_zero(body_A->velocity);
    body_A->at_rest = true;
  }
}
void resolve_collision_sphere_sphere(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  struct Sphere *sphere_A = &body_A->collider.data.sphere;
  struct Sphere *sphere_B = &body_B->collider.data.sphere;

  // Move spheres according to velocity
  float gravity = 9.8f;
  vec3 velocity_before_A, velocity_before_B;
  glm_vec3_copy(body_A->velocity, velocity_before_A);
  glm_vec3_copy(body_B->velocity, velocity_before_B);
  velocity_before_A[1] -= gravity * result.hit_time;
  velocity_before_B[1] -= gravity * result.hit_time;
  glm_vec3_muladds(velocity_before_A, result.hit_time, body_A->position);
  glm_vec3_muladds(velocity_before_B, result.hit_time, body_B->position);

  // Get world space spheres for penetration correction
  struct Sphere world_sphere_A = {0};
  glm_vec3_add(sphere_A->center, body_A->position, world_sphere_A.center);
  world_sphere_A.radius = sphere_A->radius * body_A->scale[0];

  struct Sphere world_sphere_B = {0};
  glm_vec3_add(sphere_B->center, body_B->position, world_sphere_B.center);
  world_sphere_B.radius = sphere_B->radius * body_B->scale[0];

  // Get contact normal, penetration
  vec3 difference, contact_normal;
  glm_vec3_sub(world_sphere_B.center, world_sphere_A.center, difference);
  float s = glm_vec3_norm(difference);
  float r = world_sphere_A.radius + world_sphere_B.radius;
  float penetration = (s < r) ? (r - s) + 0.001 : 0.0f;
  // float penetration = (s2 < (r * r)) ? (r - (sqrtf(s2))) + 0.001 : 0.0f;
  // Find contact normal: simply normalize the difference vector
  // (Keep it in difference instead of a new contact_normal vec3,
  // since we don't use non-normalized difference again?
  // For now this is more readable)
  glm_vec3_copy(difference, contact_normal);
  glm_vec3_normalize(contact_normal);

  if (penetration <= 0.0f) return;

  // Only correct positions and reflect velocities if the bodies aren't moving away from each other
  vec3 rel_v;
  glm_vec3_sub(velocity_before_B, velocity_before_A, rel_v);
  float separating_v = glm_dot(rel_v, contact_normal);
  if (separating_v < 0.0f){
    glm_vec3_muladds(contact_normal, (penetration / 2), body_B->position);
    glm_vec3_mulsubs(contact_normal, (penetration / 2), body_A->position);

    float restitution = 1.0f;
    float v_dot_n_A = glm_dot(velocity_before_A, contact_normal);
    float v_dot_n_B = glm_dot(velocity_before_B, contact_normal);

    // For a perfectly elastic collision,
    // swap the bodies' velocities along the contact normal
    vec3 normal_v_A, normal_v_B;
    glm_vec3_scale(contact_normal, v_dot_n_A, normal_v_A);
    glm_vec3_scale(contact_normal, v_dot_n_B, normal_v_B);

    glm_vec3_sub(velocity_before_A, normal_v_A, body_A->velocity);
    glm_vec3_add(body_A->velocity, normal_v_B, body_A->velocity);

    glm_vec3_sub(velocity_before_B, normal_v_B, body_B->velocity);
    glm_vec3_add(body_B->velocity, normal_v_A, body_B->velocity);
  }
  else{
    // Bodies are moving away from each other, don't change velocities
    glm_vec3_copy(velocity_before_A, body_A->velocity);
    glm_vec3_copy(velocity_before_B, body_B->velocity);
  }

  // if (penetration > 0.0f){
  //   // Add normal scaled by (penetration / 2) to sphere_A position
  //   glm_vec3_muladds(contact_normal, (penetration / 2), body_A->position);
  //   // Subtract from sphere_B position
  //   glm_vec3_mulsubs(contact_normal, (penetration / 2), body_B->position);
  // }
  //
  // // Reflect velocities over contact normal (orthogonal to line tangent to contact point)
  // float restitution = 1.0f;
  // float rest_velocity_threshold = 0.1f;
  // float v_dot_n_A = glm_dot(velocity_before_A, contact_normal);
  // float v_dot_n_B = glm_dot(velocity_before_B, contact_normal);
  // vec3 reflection_A;
  // vec3 reflection_B;
  // glm_vec3_scale(contact_normal, -2.0f * v_dot_n_A * restitution, reflection_A);
  // glm_vec3_scale(contact_normal, -2.0f * v_dot_n_B * restitution, reflection_B);
  // glm_vec3_add(velocity_before_A, reflection_A, body_A->velocity);
  // glm_vec3_add(velocity_before_B, reflection_B, body_B->velocity);
  //
  // // If velocity along the normal is very small,
  // // AND the sphere is perfectly atop the other,
  // // (dot(contact_normal, gravity_normal) == -1, since the dot product of two opposite unit vectors is -1)
  // // the sphere should be at rest.
  // v_dot_n_A = glm_dot(contact_normal, body_A->velocity);
  // // How to handle the other sphere? What if sphere A is under B but should be at rest?
  // // I don't know
  // v_dot_n_B = glm_dot(contact_normal, body_B->velocity);
  // if (v_dot_n_A < 0.5 && glm_dot(contact_normal, (vec3){0.0f, -1.0f, 0.0f}) == -1){
  //   glm_vec3_zero(body_A->velocity);
  //   body_A->at_rest = true;
  // }
  // if (body_A->entity != NULL){
  //   entity_play_sound_effect(body_A->entity);
  // }
}

void resolve_collision_sphere_capsule(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){

}

void resolve_collision_sphere_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  // Might be able to merge collision resolution into one function, with helpers
  // based on types?
  struct Sphere *sphere = &body_A->collider.data.sphere;
  struct Plane *plane = &body_B->collider.data.plane;

  // First move by velocity according to hit_time, applying gravity until collision
  float gravity = 9.8f;
  vec3 velocity_before;
  glm_vec3_copy(body_A->velocity, velocity_before);
  velocity_before[1] -= gravity * result.hit_time;
  glm_vec3_muladds(velocity_before, result.hit_time, body_A->position);

  // Correct penetration
  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_A->position, world_sphere.center);
  world_sphere.radius = sphere->radius * body_A->scale[0];

  float s = glm_dot(world_sphere.center, plane->normal) - plane->distance;
  float n_dot_v = glm_dot(body_A->velocity, plane->normal);
  float penetration = (s < world_sphere.radius) ? (world_sphere.radius - s) + 0.001 : 0.0f;
  if (penetration > 0.0f) {
    vec3 correction;
    glm_vec3_scale(plane->normal, penetration, correction);
    glm_vec3_add(body_A->position, correction, body_A->position);
  }

  // Reflect velocity over normal
  // float restitution = 1.0f;
  float rest_velocity_threshold = 0.1f;
  float v_dot_n = glm_dot(velocity_before, plane->normal);
  vec3 reflection;
  glm_vec3_scale(plane->normal, -2.0f * v_dot_n * body_A->restitution, reflection);
  glm_vec3_add(velocity_before, reflection, body_A->velocity);

  // If velocity along the normal is very small,
  // and the normal is opposite gravity, stop (eventually, spheres should be able to roll)
  v_dot_n = glm_dot(plane->normal, body_A->velocity);
  if (v_dot_n < 0.5 && glm_dot(plane->normal, (vec3){0.0f, -1.0f, 0.0f}) < 0){
    float distance_to_plane = glm_dot(body_A->position, plane->normal) - plane->distance;
    glm_vec3_zero(body_A->velocity);
    body_A->at_rest = true;
  }
  if (body_A->entity != NULL){
    // entity_play_sound_effect(body_A->entity);
  }
}

void resolve_collision_capsule_capsule(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){

}

void resolve_collision_capsule_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  struct Capsule *capsule = &body_A->collider.data.capsule;
  struct Plane *plane = &body_B->collider.data.plane;

  // First move by velocity according to hit_time, applying gravity until collision
  float gravity = 9.8f;
  vec3 velocity_before;
  glm_vec3_copy(body_A->velocity, velocity_before);
  velocity_before[1] -= gravity * result.hit_time;
  glm_vec3_muladds(velocity_before, result.hit_time, body_A->position);

  // Get world space capsule
  struct Capsule world_capsule = {0};
  struct Plane world_plane = {0};
  // Apply world transform to capsule segments
  if (body_A->scene_node){
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
    glm_mat3_mulv(rotation_mat3, plane->normal, world_plane.normal);
    glm_vec3_normalize(world_plane.normal);
    world_plane.distance = (plane->distance) + glm_vec3_dot(world_position, world_plane.normal);
  }

  glm_vec3_scale(capsule->segment_A, body_A->scale[0], world_capsule.segment_A);
  glm_vec3_scale(capsule->segment_B, body_A->scale[0], world_capsule.segment_B);
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  glm_mat3_mulv(rotationA, world_capsule.segment_A, world_capsule.segment_A);
  glm_mat3_mulv(rotationA, world_capsule.segment_B, world_capsule.segment_B);

  glm_vec3_add(world_capsule.segment_A, body_A->position, world_capsule.segment_A);
  glm_vec3_add(world_capsule.segment_B, body_A->position, world_capsule.segment_B);
  world_capsule.radius = capsule->radius * body_A->scale[0];

  // Correct penetration
  float n_dot_A = glm_dot(world_plane.normal, world_capsule.segment_A);
  vec3 segment;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  float n_dot_segment = glm_dot(world_plane.normal, segment);
  vec3 closest_point;
  float t = (world_plane.distance - n_dot_A) / n_dot_segment;
  if (t <= 0) glm_vec3_copy(world_capsule.segment_A, closest_point);
  else if (t >= 1) glm_vec3_copy(world_capsule.segment_B, closest_point);
  else glm_vec3_lerp(world_capsule.segment_A, world_capsule.segment_B, t, closest_point);

  float s = glm_dot(closest_point, world_plane.normal) - world_plane.distance;
  float n_dot_v = glm_dot(body_A->velocity, world_plane.normal);
  float penetration = (s < world_capsule.radius) ? (world_capsule.radius - s) + 0.001 : 0.0f;

  if (penetration <= 0.0f) return;

  vec3 correction;
  glm_vec3_scale(world_plane.normal, penetration, correction);
  glm_vec3_add(body_A->position, correction, body_A->position);
  // glm_vec3_add(body_A->position, correction, body_A->position);

  // Reflect velocity over normal
  // float restitution = 0.8f;
  float rest_velocity_threshold = 0.1f;
  float v_dot_n = glm_dot(velocity_before, world_plane.normal);
  vec3 reflection;
  glm_vec3_scale(world_plane.normal, -2.0f * v_dot_n * body_A->restitution, reflection);
  glm_vec3_add(velocity_before, reflection, body_A->velocity);

  // If velocity along the normal is very small,
  // and the normal is opposite gravity, stop (eventually, spheres should be able to roll)
  v_dot_n = glm_dot(world_plane.normal, body_A->velocity);
  if (v_dot_n < 0.5 && glm_dot(world_plane.normal, (vec3){0.0f, -1.0f, 0.0f}) < 0){
    float distance_to_plane = glm_dot(body_A->position, world_plane.normal) - world_plane.distance;
    glm_vec3_zero(body_A->velocity);
    body_A->at_rest = true;
  }
  if (body_A->entity != NULL){
    // entity_play_sound_effect(body_A->entity);
  }
}
