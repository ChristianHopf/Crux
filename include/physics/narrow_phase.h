#pragma once

#include <cglm/cglm.h>
#include <stdbool.h>
#include "physics/world.h"

#define NUM_COLLIDER_TYPES 4

struct CollisionResult {
  float hit_time;
  float penetration;
  vec3 point_of_contact;
  bool colliding;
};

typedef struct CollisionResult (*NarrowPhaseFunction)(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float delta_time);

// Move table definition to distance.c, but put an extern here.
// Do this for static things in other files too
extern NarrowPhaseFunction narrow_phase_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES];

struct CollisionResult narrow_phase_AABB_AABB(struct PhysicsBody *body_AABB_A, struct PhysicsBody *body_AABB_B, float delta_time);
struct CollisionResult narrow_phase_AABB_sphere(struct PhysicsBody *body_AABB, struct PhysicsBody *body_sphere, float delta_time);
struct CollisionResult narrow_phase_AABB_capsule(struct PhysicsBody *body_AABB, struct PhysicsBody *body_capsule, float delta_time);
struct CollisionResult narrow_phase_AABB_plane(struct PhysicsBody *body_AABB, struct PhysicsBody *body_plane, float delta_time);
struct CollisionResult narrow_phase_sphere_sphere(struct PhysicsBody *body_sphere_A, struct PhysicsBody *body_sphere_B, float delta_time);
struct CollisionResult narrow_phase_sphere_capsule(struct PhysicsBody *body_sphere, struct PhysicsBody *body_capsule, float delta_time);
struct CollisionResult narrow_phase_sphere_plane(struct PhysicsBody *body_sphere, struct PhysicsBody *body_plane, float delta_time);
struct CollisionResult narrow_phase_capsule_capsule(struct PhysicsBody *body_capsule_A, struct PhysicsBody *body_capsule_B, float delta_time);
struct CollisionResult narrow_phase_capsule_plane(struct PhysicsBody *body_capsule, struct PhysicsBody *body_plane, float delta_time);

// Helper tests
bool ray_intersect_AABB(vec3 p, vec3 d, struct AABB *e, float *hit_time, float end_time);
