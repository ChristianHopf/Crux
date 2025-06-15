#pragma once

#include <stdio.h>
#include "physics/aabb.h"

struct AABB;

void print_aabb(struct AABB *aabb);
void print_plane(struct Plane *plane);
