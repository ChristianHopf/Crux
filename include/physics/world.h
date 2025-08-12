#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include "collider.h"
#include "physics/utils.h"
#include "entity.h"

// Forward declaration to avoid redefinition of EntityType from scene including entity
struct SceneNode {
  unsigned int ID;
  mat4 local_transform;
  mat4 world_transform;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  struct Entity *entity;
  struct SceneNode *parent_node;
  struct SceneNode **children;
  unsigned int num_children;
};

struct PhysicsBody {
  // Collision
  struct Collider collider;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;
  float restitution;

  bool at_rest;

  // Associated entity
  struct Entity *entity;
  struct SceneNode *scene_node;

  // Debug rendering
  GLuint VAO, VBO, EBO;
};

struct PhysicsWorld {
  struct PhysicsBody *static_bodies;
  struct PhysicsBody *dynamic_bodies;
  struct PhysicsBody *player_bodies;
  unsigned int num_static_bodies;
  unsigned int num_dynamic_bodies;
  unsigned int num_player_bodies;
};


// World, bodies
struct PhysicsWorld *physics_world_create();
struct PhysicsBody  *physics_add_body(struct PhysicsWorld *physics_world, struct SceneNode *scene_node, struct Entity *entity, struct Collider collider, float restitution, bool dynamic);
struct PhysicsBody *physics_add_player(struct PhysicsWorld *physics_world, struct Entity *entity, struct Collider collider);

void physics_step(struct PhysicsWorld *physics_world, float delta_time);
void physics_sync_entities(struct PhysicsWorld *physics_world);

bool interval_collision(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float start_time, float end_time, float *hit_time);

float maximum_object_movement_over_time(struct PhysicsBody *body, float start_time, float end_time);
float minimum_object_distance_at_time(struct PhysicsBody *body_A, struct PhysicsBody *body_B, float time);
