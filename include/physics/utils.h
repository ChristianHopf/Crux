#pragma once

#include <stdio.h>
#include "physics/aabb.h"

struct AABB;

void print_aabb(struct AABB *aabb);
void print_plane(struct Plane *plane);

// Distance helpers
float distance_point_to_segment(vec3 point, vec3 segment_A, vec3 segment_B);
