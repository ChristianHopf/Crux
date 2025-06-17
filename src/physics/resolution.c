#include "resolution.h"
#include "collider.h"

ResolutionFunction resolution_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_PLANE] = resolve_collision_AABB_plane,
};

void resolve_collision_AABB_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time){
  if (result.colliding && result.hit_time >= 0){
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
    // Else update body position as normal with remaining time
    else{
      float remaining_time = delta_time - result.hit_time;
      if (remaining_time > 0){
        body_A->velocity[1] -= gravity * remaining_time;
        glm_vec3_muladds(body_A->velocity, remaining_time, body_A->position);
      }
    }
  }
}
