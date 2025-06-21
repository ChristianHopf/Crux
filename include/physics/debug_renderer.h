#pragma once

#include "physics/world.h"
#include <cglm/cglm.h>
#include "render_context.h"

void physics_debug_render(struct PhysicsWorld *physics_world, struct RenderContext *context);

// Init functions
void physics_debug_renderer_init(struct PhysicsWorld *physics_world);
void physics_debug_AABB_init(struct PhysicsBody *body);
void physics_debug_sphere_init(struct PhysicsBody *body);
void physics_debug_plane_init(struct PhysicsBody *body);

// Render functions
void physics_debug_AABB_render(struct AABB *aabb, struct RenderContext *context, mat4 model);
void physics_debug_sphere_render(struct Sphere *sphere, struct RenderContext *context, mat4 model);
void physics_debug_plane_render(struct Plane *plane, struct RenderContext *context, mat4 model);
