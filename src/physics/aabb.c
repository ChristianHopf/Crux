#include "physics/aabb.h"
#include "physics/utils.h"
#include "utils.h"
#include <cglm/util.h>
#include <cglm/vec3.h>

// Merge b into a
void AABB_merge(struct AABB *a, struct AABB *b){
  // If a is not initialized, copy b into a.
  // If b is not initialized, return.
  if (!a->initialized){
    glm_vec3_copy(b->center, a->center);
    glm_vec3_copy(b->extents, a->extents);
    a->initialized = true;
    return;
  }
  if (!b->initialized){
    return;
  }

  vec3 minA, maxA;
  glm_vec3_sub(a->center, a->extents, minA);
  glm_vec3_add(a->center, a->extents, maxA);

  vec3 minB, maxB;
  glm_vec3_sub(b->center, b->extents, minB);
  glm_vec3_add(b->center, b->extents, maxB);

  vec3 minMerged;
  minMerged[0] = glm_min(minA[0], minB[0]);
  minMerged[1] = glm_min(minA[1], minB[1]);
  minMerged[2] = glm_min(minA[2], minB[2]);

  vec3 maxMerged;
  maxMerged[0] = glm_max(maxA[0], maxB[0]);
  maxMerged[1] = glm_max(maxA[1], maxB[1]);
  maxMerged[2] = glm_max(maxA[2], maxB[2]);

  vec3 centerMerged, extentsMerged;
  glm_vec3_add(minMerged, maxMerged, centerMerged);
  glm_vec3_scale(centerMerged, 0.5f, centerMerged);
  glm_vec3_sub(maxMerged, minMerged, extentsMerged);
  glm_vec3_scale(extentsMerged, 0.5f, extentsMerged);

  glm_vec3_copy(centerMerged, a->center);
  glm_vec3_copy(extentsMerged, a->extents);

  a->initialized = true;

  return;
}

// Consider when building AABBs that the center is NOT scaled.
// Defining an AABB such as:
// - center: [0, -1, 0],
// - extents: [10, 1, 10]
// - with a scale vector of [0.5, 0.5, 0.5]
// will not use a center of [0, -0.5, 0]
void AABB_update(struct AABB *src, mat3 rotation, vec3 translation, vec3 scale, struct AABB *dest){
  //Ericson's algorithm uses a row-major rotation matrix,
  // but GLM expects column-major. Swap i and j to match model rendering
  for (int i = 0; i < 3; i++){
    dest->center[i] = translation[i];
    dest->extents[i] = 0.0f;
    for (int j = 0; j < 3; j++){
      dest->center[i] += rotation[j][i] * src->center[j];
      dest->extents[i] += fabs(rotation[j][i]) * src->extents[j] * fabs(scale[i]);
    }
    // dest->extents[i] *= fabs(scale[i]);
  }

  // print_glm_vec3(src->center, "AABB UPDATE: SRC CENTER");
  // print_glm_vec3(dest->center, "AABB UPDATE: DEST CENTER");
  //
  // printf("ORIGINAL AND ROTATED AABBS\n");
  // print_aabb(src);
  // print_aabb(dest);
}

void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex){
  // For each axis, if the vertex extends the AABB in either direction,
  // update the center and radius on that axis by half that distance
  for (int i = 0; i < 3; i++){
    if (vertex[i] > (aabb->center[i] + aabb->extents[i])){
      float dist = vertex[i] - (aabb->center[i] + aabb->extents[i]);
      aabb->center[i] += dist * 0.5f;
      aabb->extents[i] += dist * 0.5f;
    }
    if (vertex[i] < (aabb->center[i] - aabb->extents[i])){
      float dist = aabb->center[i] - aabb->extents[i] - vertex[i];
      aabb->center[i] -= dist * 0.5f;
      aabb->extents[i] += dist * 0.5f;
    }
  }
}

// COLLISION TESTS
//
// Intersection between AABBs
bool AABB_intersect_AABB(struct AABB *a, struct AABB *b){
  if (fabs(a->center[0] - b->center[0]) > (a->extents[0] + b->extents[0])) return false;
  if (fabs(a->center[1] - b->center[1]) > (a->extents[1] + b->extents[1])) return false;
  if (fabs(a->center[2] - b->center[2]) > (a->extents[2] + b->extents[2])) return false;
  return true;
}

// Intersection between an AABB and a plane
bool AABB_intersect_plane(struct AABB *box, struct Plane *plane){

  // Get radius of the extents' projection interval onto the plane's normal
  float r =
    box->extents[0] * fabs(plane->normal[0]) +
    box->extents[1] * fabs(plane->normal[1]) +
    box->extents[2] * fabs(plane->normal[2]); 
  // Get distance from the center of AABB to the plane
  float s = glm_dot(plane->normal, box->center) - plane->distance;

  // If the distance from the center to the plane is within the interval,
  // the AABB is colliding with the plane
  return fabs(s) <= r;
}
