#pragma once

#include "physics/world.h"
#include "physics/narrow_phase.h"

#define NUM_COLLIDER_TYPES 3

typedef void (*ResolutionFunction)(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time);

extern ResolutionFunction resolution_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES];

// Minimum distance functions
void resolve_collision_AABB_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time);
void resolve_collision_sphere_sphere(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time);
void resolve_collision_sphere_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, struct CollisionResult result, float delta_time);
