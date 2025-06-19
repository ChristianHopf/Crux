#pragma once

#include "physics/world.h"
#include "physics/aabb.h"
#include "physics/plane.h"

// Defines a function pointer lookup table for calculating minimum distances between
// different types of volumes. I didn't want a huge switch statement, and it turns out
// you can do this in C!

#define NUM_COLLIDER_TYPES 3

typedef float (*DistanceFunction)(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time);

// Move table definition to distance.c, but put an extern here.
// Do this for static things in other files too
extern DistanceFunction distance_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES];

// Minimum distance functions
float min_dist_at_time_AABB_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time);
float min_dist_at_time_sphere_sphere(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time);
float min_dist_at_time_sphere_plane(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time);

// DistanceFunction distance_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
//   [COLLIDER_AABB][COLLIDER_PLANE] = min_dist_at_time_AABB_plane,
//   [COLLIDER_PLANE][COLLIDER_AABB] = min_dist_at_time_AABB_plane
// };
