#ifndef PHYSICS_UTILS_H
#define PHYSICS_UTILS_H

#include <stdio.h>
#include "physics/aabb.h"

struct AABB;

void print_aabb(struct AABB *aabb);
void print_plane_collider(struct PlaneCollider *plane);

#endif
