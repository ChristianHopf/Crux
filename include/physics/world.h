#pragma once

// #include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include "aabb.h"
#include "collider.h"
#include "physics/utils.h"
#include "entity.h"

struct PhysicsBody {
  // Collision
  struct Collider collider;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;
  bool at_rest;

  // Associated entity
  struct Entity *entity;

  // Debug rendering
  GLuint VAO, VBO, EBO;
};

struct PhysicsWorld {
  struct PhysicsBody *static_bodies;
  struct PhysicsBody *dynamic_bodies;
  unsigned int num_static_bodies;
  unsigned int num_dynamic_bodies;
};


// World, bodies
struct PhysicsWorld *physics_world_create();
struct PhysicsBody  *physics_add_body(struct PhysicsWorld *physics_world, struct Entity *entity, struct Collider collider, bool dynamic);

void physics_step(struct PhysicsWorld *physics_world, float delta_time);

bool interval_collision(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float start_time, float end_time, float *hit_time);

float maximum_object_movement_over_time(struct PhysicsBody *body, float start_time, float end_time);
float minimum_object_distance_at_time(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time);
