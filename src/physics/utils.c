#include "physics/utils.h"

void print_aabb(struct AABB *aabb){
  // Convert AABB to min/max format for debug printing
  // vec3 min;
  // vec3 max;
  //
  // for (int i = 0; i < 3; i++){
  //   min[i] = aabb->center[i] - aabb->extents[i];
  //   max[i] = aabb->center[i] + aabb->extents[i];
  // }

  printf("AABB {\n");
  printf("  center: (%.7f, %.7f, %.7f)\n", aabb->center[0], aabb->center[1], aabb->center[2]);
  printf("  extents: (%.7f, %.7f, %.7f)\n", aabb->extents[0], aabb->extents[1], aabb->extents[2]);
  printf("}\n");

  // printf("AABB {\n");
  // printf("  min: (%.2f, %.2f, %.2f\n)", min[0], min[1], min[2]);
  // printf("  max: (%.2f, %.2f, %.2f\n)", max[0], max[1], max[2]);
  // printf("}\n");
}

void print_plane(struct Plane *plane){
  printf("Plane:\n");
  printf("{\n");
  printf("  Normal: (%.3f, %.3f, %.3f)\n", plane->normal[0], plane->normal[1], plane->normal[2]);
  printf("  Distance: %.2f\n", plane->distance);
  printf("}\n");
}

// Implementation of algorithm as described in Point Line-Segment Distance by Two-Bit Coding
float distance_point_to_segment(vec3 point, vec3 segment_A, vec3 segment_B){
  vec3 segment, A_to_point;
  glm_vec3_sub(segment_B, segment_A, segment);
  glm_vec3_sub(point, segment_A, A_to_point);

  // Normalize projection of point onto segment
  float proj = glm_dot(segment, A_to_point);
  float d = proj / glm_dot(segment, segment);

  // Closest point is segment_A
  if (d <= 0){
    return glm_vec3_norm(A_to_point);
  }
  // Closest point is segment_B
  else if (d >= 1){
    vec3 B_to_point;
    glm_vec3_sub(point, segment_B, B_to_point);
    return glm_vec3_norm(B_to_point);
  }
  // Closest point is between segment endpoints
  else{
    vec3 d_along_segment, closest_point, P_to_closest_point;
    glm_vec3_scale(segment, d, d_along_segment);
    glm_vec3_add(segment_A, d_along_segment, closest_point);
    glm_vec3_sub(closest_point, point, P_to_closest_point);
    return glm_vec3_norm(P_to_closest_point);
  }
}
