#include "physics/world.h"

#define MAX_PHYSICS_BODIES 128

struct PhysicsWorld *physics_world_create(){
  struct PhysicsWorld *world = (struct PhysicsWorld *)calloc(1, sizeof(struct PhysicsWorld));
  if (!world){
    printf("Error: failed to allocate PhysicsWorld in physics_world_create\n");
    return NULL;
  }
  world->bodies = (struct PhysicsBody *)calloc(MAX_PHYSICS_BODIES, sizeof(struct PhysicsBody));
  printf("Created physics world\n");

  // Might want some kind of default field population later.
  // calloc should be fine for now, though.
  return world;
}

void physics_add_body(struct PhysicsWorld *physics_world, struct AABB aabb){
  printf("Physics world has %d bodies\n", physics_world->num_bodies);
  // Create body
  // struct PhysicsBody *body = (struct PhysicsBody *)malloc(sizeof(struct PhysicsBody));
  // body->aabb = *aabb;
  // printf("Created body with AABB:\n");
  // print_aabb(&body->aabb);

  // Add body to physics_world
  // physics_world->bodies[physics_world->num_bodies++] = *body;

  struct PhysicsBody body = {
    .aabb = aabb
  };

  physics_world->bodies[physics_world->num_bodies++] = body;
  printf("Allocated physics body, physics world now has %d bodies\n", physics_world->num_bodies);
  physics_world->bodies[physics_world->num_bodies].aabb = aabb;
  printf("Successfully added body to physics_world\n");
}
