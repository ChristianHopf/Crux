#include "resolution.h"
#include "collider.h"

ResolutionFunction resolution_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_PLANE] = resolve_collision_AABB_plane,
  [COLLIDER_SPHERE][COLLIDER_SPHERE] = resolve_collision_sphere_sphere,
  [COLLIDER_SPHERE][COLLIDER_PLANE] = resolve_collision_sphere_plane
};

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
  glm_euler_xyz(body_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);

  vec3 translationA, scaleA;
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
  float restitution = 1.0f;
  float rest_velocity_threshold = 0.1f;
  float v_dot_n = glm_dot(velocity_before, plane->normal);
  vec3 reflection;
  glm_vec3_scale(plane->normal, -2.0f * v_dot_n * restitution, reflection);
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

  vec3 difference, contact_normal;
  glm_vec3_sub(world_sphere_A.center, world_sphere_B.center, difference);
  float s2 = glm_dot(difference, difference);
  float r = world_sphere_A.radius + world_sphere_B.radius;
  float penetration = (s2 < (r * r)) ? (r - s2) + 0.001 : 0.0f;
  // Find contact normal: simply normalize the difference vector
  // Keep it in difference instead of a new contact_normal vec3,
  // since we don't use non-normalized difference again?
  // For now this is more readable
  glm_vec3_copy(difference, contact_normal);
  glm_vec3_normalize(contact_normal);
  if (penetration > 0.0f){
    // Add normal scaled by (penetration / 2) to sphere_A position
    glm_vec3_muladds(contact_normal, (penetration / 2), body_A->position);
    // Subtract from sphere_B position
    glm_vec3_mulsubs(contact_normal, (penetration / 2), body_B->position);
  }

  // Reflect velocities over contact normal (orthogonal to line tangent to contact point)
  float restitution = 1.0f;
  float rest_velocity_threshold = 0.1f;
  float v_dot_n_A = glm_dot(velocity_before_A, contact_normal);
  float v_dot_n_B = glm_dot(velocity_before_B, contact_normal);
  vec3 reflection_A;
  vec3 reflection_B;
  glm_vec3_scale(contact_normal, -2.0f * v_dot_n_A * restitution, reflection_A);
  glm_vec3_scale(contact_normal, -2.0f * v_dot_n_B * restitution, reflection_B);
  glm_vec3_add(velocity_before_A, reflection_A, body_A->velocity);
  glm_vec3_add(velocity_before_B, reflection_B, body_B->velocity);

  print_glm_vec3(body_A->velocity, "REFLECTED SPHERE A VELOCITY");
  print_glm_vec3(body_B->velocity, "REFLECTED SPHERE B VELOCITY");

  // If velocity along the normal is very small,
  // AND the sphere is perfectly atop the other,
  // (dot(contact_normal, gravity_normal) == -1, since the dot product of two opposite unit vectors is -1)
  // the sphere should be at rest.
  v_dot_n_A = glm_dot(contact_normal, body_A->velocity);
  // How to handle the other sphere? What if sphere A is under B but should be at rest?
  // I don't know
  v_dot_n_B = glm_dot(contact_normal, body_B->velocity);
  if (v_dot_n_A < 0.5 && glm_dot(contact_normal, (vec3){0.0f, -1.0f, 0.0f}) == -1){
    // float distance_to_plane = glm_dot(body_A->position, normal) - plane->distance;
    glm_vec3_zero(body_A->velocity);
    body_A->at_rest = true;
  }
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
  // glm_vec3_scale(world_sphere.center, body_A->scale[0], world_sphere.center);
  world_sphere.radius = sphere->radius * body_A->scale[0];

  float s = glm_dot(world_sphere.center, plane->normal) - plane->distance;
  float n_dot_v = glm_dot(body_A->velocity, plane->normal);
  float penetration = (s < world_sphere.radius) ? (world_sphere.radius - s) + 0.001 : 0.0f;
  if (penetration > 0.0f) {
    printf("Penetration of %f with s %f, sphere radius %f\n", penetration, s, world_sphere.radius);
    vec3 correction;
    glm_vec3_scale(plane->normal, penetration, correction);
    glm_vec3_add(body_A->position, correction, body_A->position);
  }

  // Reflect velocity over normal
  float restitution = 1.0f;
  float rest_velocity_threshold = 0.1f;
  float v_dot_n = glm_dot(velocity_before, plane->normal);
  vec3 reflection;
  glm_vec3_scale(plane->normal, -2.0f * v_dot_n * restitution, reflection);
  glm_vec3_add(velocity_before, reflection, body_A->velocity);

  // If velocity along the normal is very small,
  // and the normal is opposite gravity, stop (eventually, spheres should be able to roll)
  v_dot_n = glm_dot(plane->normal, body_A->velocity);
  if (v_dot_n < 0.5 && glm_dot(plane->normal, (vec3){0.0f, -1.0f, 0.0f}) < 0){
    float distance_to_plane = glm_dot(body_A->position, plane->normal) - plane->distance;
    glm_vec3_zero(body_A->velocity);
    body_A->at_rest = true;
  }
}
