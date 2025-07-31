#include <stdbool.h>
#include "physics/narrow_phase.h"

#define EPSILON 0.0001

NarrowPhaseFunction narrow_phase_functions[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] = {
  [COLLIDER_AABB][COLLIDER_AABB] = narrow_phase_AABB_AABB,
  [COLLIDER_AABB][COLLIDER_SPHERE] = narrow_phase_AABB_sphere,
  [COLLIDER_AABB][COLLIDER_CAPSULE] = narrow_phase_AABB_capsule,
  [COLLIDER_AABB][COLLIDER_PLANE] = narrow_phase_AABB_plane,
  [COLLIDER_SPHERE][COLLIDER_SPHERE] = narrow_phase_sphere_sphere,
  [COLLIDER_SPHERE][COLLIDER_CAPSULE] = narrow_phase_sphere_capsule,
  [COLLIDER_SPHERE][COLLIDER_PLANE] = narrow_phase_sphere_plane,
  [COLLIDER_SPHERE][COLLIDER_CAPSULE] = narrow_phase_sphere_capsule,
  [COLLIDER_CAPSULE][COLLIDER_CAPSULE] = narrow_phase_capsule_capsule,
  [COLLIDER_CAPSULE][COLLIDER_PLANE] = narrow_phase_capsule_plane
};

struct CollisionResult narrow_phase_AABB_AABB(struct PhysicsBody *body_AABB_A, struct PhysicsBody *body_AABB_B, float delta_time){
  struct AABB *aabb_A = &body_AABB_A->collider.data.aabb;
  struct AABB *aabb_B = &body_AABB_B->collider.data.aabb;
  struct CollisionResult result = {0};

  // Get world space bodies
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_AABB_A->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  vec3 translationA;
  glm_vec3_copy(body_AABB_A->position, translationA);
  vec3 scaleA;
  glm_vec3_copy(body_AABB_A->scale, scaleA);
  struct AABB world_AABB_A = {0};
  AABB_update(aabb_A, rotationA, translationA, scaleA, &world_AABB_A);

  // Get world space bodies
  mat4 eulerB;
  mat3 rotationB;
  glm_euler_xyz(body_AABB_B->rotation, eulerB);
  glm_mat4_pick3(eulerB, rotationB);
  vec3 translationB;
  glm_vec3_copy(body_AABB_B->position, translationB);
  vec3 scaleB;
  glm_vec3_copy(body_AABB_B->scale, scaleB);
  struct AABB world_AABB_B = {0};
  AABB_update(aabb_B, rotationB, translationB, scaleB, &world_AABB_B);

  // If already intersecting, they're already colliding
  if (AABB_intersect_AABB(&world_AABB_A, &world_AABB_B)){
    result.hit_time = 0;
    result.colliding = true;
    return result;
  }
  else{
    vec3 rel_v;
    glm_vec3_sub(body_AABB_A->velocity, body_AABB_B->velocity, rel_v);

    vec3 min_A, max_A;
    vec3 min_B, max_B;
    glm_vec3_sub(world_AABB_A.center, world_AABB_A.extents, min_A);
    glm_vec3_add(world_AABB_A.center, world_AABB_A.extents, max_A);
    glm_vec3_sub(world_AABB_B.center, world_AABB_B.extents, min_B);
    glm_vec3_add(world_AABB_B.center, world_AABB_B.extents, max_B);

    float t_first = 0.0f;
    float t_last = delta_time;

    // Find first and last times of contact on each axis
    for (int i = 0; i < 3; i++){
      if (rel_v[i] < 0.0f){
        // If rel_v is negative, and a is already past b on this axis, exit
        if (max_A[i] < min_B[i]){
          result.hit_time = -1;
          result.colliding = false;
          return result;
        }
        if (min_A[i] > max_B[i]){
          t_first = fmaxf((max_B[i] - min_A[i]) / rel_v[i], t_first);
        }
        if(max_A[i] > min_B[i]){
          t_last = fminf((min_B[i] - max_A[i]) / rel_v[i], t_last);
        }
      }
      if (rel_v[i] > 0.0f){
        // If rel_v is positive, and a is already past b on this axis, exit
        if (min_A[i] > max_B[i]){
          result.hit_time = -1;
          result.colliding = false;
          return result;
        }
        if (max_A[i] < min_B[i]){
          t_first = fmaxf((min_B[i] - max_A[i]) / rel_v[i], t_first);
        }
        if(min_A[i] < max_B[i]){
          t_last = fminf((max_B[i] - min_A[i]) / rel_v[i], t_last);
        }
      }

      // If the greatest time of first contact occurs after the smallest time of last contact,
      // then we have to leave a slab to enter another, which means we don't collide with
      // the intersecting volume.
      if (t_first > t_last){
        result.hit_time = -1;
        result.colliding = false;
        return result;
      }
    }
    
    // Intersection confirmed on all 3 axes
    result.hit_time = t_first;
    result.colliding = true;
    return result;
  }
}

struct CollisionResult narrow_phase_AABB_sphere(struct PhysicsBody *body_AABB, struct PhysicsBody *body_sphere, float delta_time){
  struct AABB *box = &body_AABB->collider.data.aabb;
  struct Sphere *sphere = &body_sphere->collider.data.sphere;
  struct CollisionResult result = {0};

  // Get world space bodies
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_AABB->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  vec3 translationA;
  glm_vec3_copy(body_AABB->position, translationA);
  vec3 scaleA;
  glm_vec3_copy(body_AABB->scale, scaleA);
  struct AABB world_AABB = {0};
  AABB_update(box, rotationA, translationA, scaleA, &world_AABB);

  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_sphere->position, world_sphere.center);
  world_sphere.radius = sphere->radius * body_sphere->scale[0];

  // Get the AABB e that bounds the world_AABB swept by the sphere
  struct AABB e = world_AABB;
  glm_vec3_adds(e.extents, world_sphere.radius, e.extents);

  vec3 rel_v;
  glm_vec3_sub(body_sphere->velocity, body_AABB->velocity, rel_v);

  // LAZY VERSION: Assume sphere direction ray intersection with the bounding AABB e an intersection with the world AABB.
  // Come back to this later for more precise edge and corner testing and point of contact.
  // vec3 p;
  float t_min;
  bool intersect = ray_intersect_AABB(world_sphere.center, rel_v, &e, &t_min, delta_time);
  if (!intersect || t_min > delta_time){
    result.hit_time = -1;
    result.colliding = false;
  }
  else{
    result.hit_time = t_min;
    result.colliding = true;
  }

  return result;

  // else{
  //   // Bit flags for the faces outside of which the point p of intersection between sphere center and AABB e lies
  //   // -u: min, v: max
  //   // Ex: u = 6, v = 1 => The sphere lies on the min side of e's y and z extents, and the max size of its x extent.
  //   int u, v = 0;
  //   vec3 box_min, box_max;
  //   glm_vec3_sub(world_AABB.center, world_AABB.extents, box_min);
  //   glm_vec3_add(world_AABB.center, world_AABB.extents, box_max);
  //   if (p[0] < box_min[0]) u |= 1;
  //   if (p[0] > box_max[0]) v |= 1;
  //   if (p[1] < box_min[1]) u |= 2;
  //   if (p[1] > box_max[1]) v |= 2;
  //   if (p[2] < box_min[2]) u |= 4;
  //   if (p[2] > box_max[2]) v |= 4;
  //
  //   // Bit mask m = u OR v
  //   int m = u + v;
  //
  //   // Line segment
  //
  //   // If all 3 bits were set, p is in some vertex region
  //   if (m == 7){
  //
  //   }
  //   // If only 1 bit was set, p is in some face region
  //   else if ((m & (m - 1)) == 0){
  //
  //   }
  //   // Else, p is in some edge region. Intersect with the edge capsule
  //   else{
  //
  //   }
  // }
}

struct CollisionResult narrow_phase_AABB_capsule(struct PhysicsBody *body_AABB, struct PhysicsBody *body_capsule, float delta_time){

}

struct CollisionResult narrow_phase_AABB_plane(struct PhysicsBody *body_AABB, struct PhysicsBody *body_plane, float delta_time){
  struct AABB *box = &body_AABB->collider.data.aabb;
  struct Plane *plane = &body_plane->collider.data.plane;
  struct CollisionResult result = {0};

  // Get a world space version of the current AABB
  mat4 eulerA;
  mat3 rotationA;
  glm_euler_xyz(body_AABB->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);

  vec3 translationA;
  glm_vec3_copy(body_AABB->position, translationA);
  vec3 scaleA;
  glm_vec3_copy(body_AABB->scale, scaleA);
  struct AABB worldAABB_A = {0};
  AABB_update(box, rotationA, translationA, scaleA, &worldAABB_A);

  // Get relative velocity
  vec3 rel_v;
  glm_vec3_copy(body_AABB->velocity, rel_v);

  // Get radius of projection interval
  float r =
    worldAABB_A.extents[0] * fabs(plane->normal[0]) +
    worldAABB_A.extents[1] * fabs(plane->normal[1]) +
    worldAABB_A.extents[2] * fabs(plane->normal[2]); 

  // Get distance from center of AABB to plane
  // printf("Dot product of plane normal and world aabb center %f\n", glm_dot(plane->normal, worldAABB_A.center));
  // printf("Plane distance from origin: %f\n", plane->distance);
  float s = glm_dot(plane->normal, worldAABB_A.center) - plane->distance;
  // printf("Distance from center to plane: %f\n", s);

  // Get dot product of normal and relative velocity
  // - n*v = 0 => moving parallel
  // - n*v < 0 => moving towards plane
  // - n*v > 0 => moving away from the plane
  float n_dot_v = glm_dot(plane->normal, rel_v);
  // printf("r: %f, s: %f, n_dot_v: %f\n", r, s, n_dot_v);

  // n*v == 0 => parallel movement
  if (n_dot_v == 0){
    if (fabs(s) <= r){
      result.colliding = true;
      result.hit_time = 0;
    }
    else{
      result.colliding = false;
      result.hit_time = -1;
    }
  }
  // If n*v != 0, solve for t.
  // Ericson's equation:
  // t = (r + d - (n * C)) / (n * v)
  // Is equivalent to:
  // t = (r - ((n * C) - d)) / (n * v), or
  // t = (r - s) / (n * v)
  else {
    // Moving towards plane
    if (n_dot_v < 0){
      result.hit_time = (r - s) / -n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }
    // Moving away from plane
    else{
      // Already intersecting
      if (fabs(s) <= r){
        result.hit_time = 0;
        result.colliding = true;
      } else {
        result.hit_time = (r + s) / -n_dot_v;
        result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
        // printf("Moving away result hit time %f\n", result.hit_time);
      }
    }

    // Sometimes hit_time is bigger than delta_time. Clamp to delta_time
    if (result.colliding){
      result.hit_time = fmaxf(0.0f, fminf(result.hit_time, delta_time));
    }

    // If hit_time is outside of time interval,
    // no collision unless s is within r
    if (!result.colliding){
      if (fabs(s) <= r){
    result.hit_time = 0;
        result.colliding = true;
      }
      else{
        result.hit_time = -1;
      }
    } 

    // Point of contact Q = C(t) - rn
    // vec3 Q;
    // if (result.colliding){
    //   glm_vec3_copy(worldAABB_A.center, result.point_of_contact);
    //   glm_vec3_muladds(body_AABB->velocity, result.hit_time, result.point_of_contact);
    //
    //   if (fabs(s) < r){
    //     float dist = glm_dot(plane->normal, result.point_of_contact) - plane->distance;
    //     result.penetration = dist;
    //     // printf("Result penetration %f\n", s - dist);
    //     glm_vec3_mulsubs(plane->normal, dist, result.point_of_contact);
    //   }
    // }
    // glm_vec3_copy(worldAABB_A.center, result.point_of_contact);
    // glm_vec3_muladds(body_AABB->velocity, result.hit_time, result.point_of_contact);
    // glm_vec3_mulsubs(plane->normal, r, result.point_of_contact);
  }

  return result;
}

struct CollisionResult narrow_phase_sphere_sphere(struct PhysicsBody *body_sphere_A, struct PhysicsBody *body_sphere_B, float delta_time){
  struct Sphere *sphere_A = &body_sphere_A->collider.data.sphere;
  struct Sphere *sphere_B = &body_sphere_B->collider.data.sphere;
  struct CollisionResult result = {0};

  // Get world space spheres
  struct Sphere world_sphere_A = {0};
  glm_vec3_add(sphere_A->center, body_sphere_A->position, world_sphere_A.center);
  world_sphere_A.radius = sphere_A->radius * body_sphere_A->scale[0];

  struct Sphere world_sphere_B = {0};
  glm_vec3_add(sphere_B->center, body_sphere_B->position, world_sphere_B.center);
  world_sphere_B.radius = sphere_B->radius * body_sphere_B->scale[0];

  // s - difference between centers
  // v - relative velocity
  // r - sum of radii
  vec3 s, v;
  float r;
  glm_vec3_sub(world_sphere_A.center, world_sphere_B.center, s);
  glm_vec3_sub(body_sphere_A->velocity, body_sphere_B->velocity, v);
  r = world_sphere_A.radius + world_sphere_B.radius;

  // If the distance between spheres is less than the sum of their radii,
  // they're already colliding
  float c = glm_dot(s, s) - (r * r);
  if (c < 0.0f){
    result.hit_time = 0;
    result.colliding = true;
  }
  else{
    // If the magnitude of the relative velocity is 0,
    // the spheres aren't moving relative to each other
    float a = glm_dot(v, v);
    if (a < EPSILON){
      result.hit_time = -1;
      result.colliding = false;
    }

    // If the projection of the relative velocity on the difference vector is positive,
    // the spheres are moving away from each other.
    float b = glm_dot(v, s);
    if (b >= 0.0f){
      result.hit_time = -1;
      result.colliding = false;
    }

    // If there is no real-valued root for the discriminant of the quadratic equation for t,
    // the spheres do not collide
    float d = (b * b) - a * c;
    if (d < 0.0f){
      result.hit_time = -1;
      result.colliding = false;
    }

    // Two solutions for t, we want the first (smaller) one
    result.hit_time = (-b - sqrt(d)) / a;
    result.colliding = true;
  }

  return result;
}

struct CollisionResult narrow_phase_sphere_capsule(struct PhysicsBody *body_sphere, struct PhysicsBody *body_capsule, float delta_time){

}

struct CollisionResult narrow_phase_sphere_plane(struct PhysicsBody *body_sphere, struct PhysicsBody *body_plane, float delta_time){
  struct Sphere *sphere = &body_sphere->collider.data.sphere;
  struct Plane *plane = &body_plane->collider.data.plane;
  struct CollisionResult result = {0};

  // Get world space sphere
  struct Sphere world_sphere = {0};
  glm_vec3_add(sphere->center, body_sphere->position, world_sphere.center);
  // glm_vec3_muladds(body_A->velocity, time, world_sphere.center);
  // glm_vec3_scale(world_sphere.center, body_sphere->scale[0], world_sphere.center);
  world_sphere.radius = sphere->radius * body_sphere->scale[0];

  // Get signed distance from sphere center to plane
  float s = glm_dot(world_sphere.center, plane->normal) - plane->distance;
  // printf("Signed distance from sphere center to plane: %f\n", s);

  // Get velocity along normal
  float n_dot_v = glm_dot(body_sphere->velocity, plane->normal);

  // Compute product of signed distance and normal velocity
  float discriminant = s * n_dot_v;

  // Solve for t based on plane displacement according to the sign of:
  // (n * V)(n * C - d)
  // - term > 0 => sphere is moving away from the plane
  // - term < 0 => sphere is moving towards the plane
  // - term == 0 => sphere is moving parallel to the plane
  if (discriminant == 0){
    if (fabs(s) <= world_sphere.radius){
      result.hit_time = 0;
      result.colliding = true;
    }
    else{
      result.hit_time = -1;
      result.colliding = false;
    }
  }
  else{
    if (discriminant < 0){
      // t = (r - ((n * C) - d)) / (n * v)
      result.hit_time = (world_sphere.radius - s) / n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }
    else{
      // t = (-r - ((n * C) - d)) / (n * v)
      result.hit_time = (-world_sphere.radius - s) / n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }

    // If hit_time is outside of interval, check if already colliding
    if (!result.colliding){
      if (abs(s <= world_sphere.radius)){
        result.hit_time = 0;
        result.colliding = true;
      }
      else{
        result.hit_time = -1;
      }
    }
  }

  return result;
}

struct CollisionResult narrow_phase_capsule_capsule(struct PhysicsBody *body_capsule_A, struct PhysicsBody *body_capsule_B, float delta_time){

}

struct CollisionResult narrow_phase_capsule_plane(struct PhysicsBody *body_capsule, struct PhysicsBody *body_plane, float delta_time){
  struct Capsule *capsule = &body_capsule->collider.data.capsule;
  struct Plane *plane = &body_plane->collider.data.plane;
  struct CollisionResult result = {0};

  // Get world space capsule
  struct Capsule world_capsule = {0};
  struct Plane world_plane = {0};
  // Apply world transform to capsule segments
  if (body_capsule->scene_node){
    glm_mat4_mulv3(body_capsule->scene_node->world_transform, capsule->segment_A, 1.0f, world_capsule.segment_A);
    glm_mat4_mulv3(body_capsule->scene_node->world_transform, capsule->segment_B, 1.0f, world_capsule.segment_B);
  }
  if (body_plane->scene_node){
    mat3 rotation_mat3;
    vec3 world_position, world_rotation, world_scale;
    // Plane normal and distance include an already-applied local transformation
    glm_mat4_mulv3(body_plane->scene_node->parent_node->world_transform, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, world_position);
    glm_decompose_scalev(body_plane->scene_node->parent_node->world_transform, world_scale);
    glm_mat4_pick3(body_plane->scene_node->parent_node->world_transform, rotation_mat3);
    if (world_scale[0] != 0.0f){
      glm_mat3_scale(rotation_mat3, 1.0f / world_scale[0]);
    }

    // Transform plane
    print_glm_vec3(plane->normal, "Wall plane normal before transformation");
    print_glm_mat3(rotation_mat3, "Rotation mat3");
    glm_mat3_mulv(rotation_mat3, plane->normal, world_plane.normal);
    glm_vec3_normalize(world_plane.normal);
    world_plane.distance = (plane->distance) + glm_vec3_dot(world_position, world_plane.normal);
    if (plane->distance == -5){
      print_glm_vec3(plane->normal, "Wall plane normal");
      print_glm_vec3(world_plane.normal, "World wall plane normal");
    }
  }

  glm_vec3_scale(capsule->segment_A, body_capsule->scale[0], world_capsule.segment_A);
  glm_vec3_scale(capsule->segment_B, body_capsule->scale[0], world_capsule.segment_B);
  mat4 eulerA;
  mat3 rotationA;
  vec3 rotatedA, rotatedB;
  glm_euler_xyz(body_capsule->rotation, eulerA);
  glm_mat4_pick3(eulerA, rotationA);
  glm_mat3_mulv(rotationA, world_capsule.segment_A, world_capsule.segment_A);
  glm_mat3_mulv(rotationA, world_capsule.segment_B, world_capsule.segment_B);

  glm_vec3_add(world_capsule.segment_A, body_capsule->position, world_capsule.segment_A);
  glm_vec3_add(world_capsule.segment_B, body_capsule->position, world_capsule.segment_B);
  world_capsule.radius = capsule->radius * body_capsule->scale[0];

  // Get distance from closest point on capsule to plane
  float n_dot_A = glm_dot(world_plane.normal, world_capsule.segment_A);
  vec3 segment;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, segment);
  float n_dot_segment = glm_dot(world_plane.normal, segment);

  vec3 closest_point;
  float t = (world_plane.distance - n_dot_A) / n_dot_segment;
  if (t <= 0) glm_vec3_copy(world_capsule.segment_A, closest_point);
  else if (t >= 1) glm_vec3_copy(world_capsule.segment_B, closest_point);
  else glm_vec3_lerp(world_capsule.segment_A, world_capsule.segment_B, t, closest_point);

  float s = glm_dot(closest_point, world_plane.normal) - world_plane.distance;

  // Get velocity along normal
  float n_dot_v = glm_dot(body_capsule->velocity, world_plane.normal);

  // Compute product of signed distance and normal velocity
  float discriminant = s * n_dot_v;

  // Solve for t based on plane displacement according to the sign of:
  // (n * V)(n * C - d)
  // - term > 0 => capsule is moving away from the plane
  // - term < 0 => capsule is moving towards the plane
  // - term == 0 => capsule is moving parallel to the plane
  if (discriminant == 0){
    if (fabs(s) <= world_capsule.radius){
      result.hit_time = 0;
      result.colliding = true;
    }
    else{
      result.hit_time = -1;
      result.colliding = false;
    }
  }
  else{
    if (discriminant < 0){
      // t = (r - ((n * C) - d)) / (n * v)
      result.hit_time = (world_capsule.radius - s) / n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }
    else{
      // t = (-r - ((n * C) - d)) / (n * v)
      result.hit_time = (-world_capsule.radius - s) / n_dot_v;
      result.colliding = (result.hit_time >= 0 && result.hit_time <= delta_time);
    }

    // If hit_time is outside of interval, check if already colliding
    if (!result.colliding){
      if (abs(s <= world_capsule.radius)){
        result.hit_time = 0;
        result.colliding = true;
      }
      else{
        result.hit_time = -1;
      }
    }
  }

  return result;
}


// HELPERS
bool ray_intersect_AABB(vec3 p, vec3 d, struct AABB *aabb, float *hit_time, float end_time){
  *hit_time = 0.0f;
  float t_max = end_time;

  // Get min and max
  vec3 box_min, box_max;
  glm_vec3_sub(aabb->center, aabb->extents, box_min);
  glm_vec3_add(aabb->center, aabb->extents, box_max);

  // Three slabs (pairs of parallel box faces)
  for(int i = 0; i < 3; i++){
    // Relative velocity on this axis is parallel to its slab.
    // If it doesn't intersect on this axis, it doesn't intersect
    if (fabsf(d[i]) < EPSILON){
      if (p[i] < box_min[i] || p[i] > box_max[i]) return false;
    }
    else{
      // t1: time to intersect the closer end of this slab
      // t2: time to intersect the farther end of this slab
      float t1 = (box_min[i] - p[i]) / d[i];
      float t2 = (box_max[i] - p[i]) / d[i];
      if (t1 > t2){
        float temp = t1;
        t1 = t2;
        t2 = temp;
      }

      // Intersection of slab intersection intervals:
      // The segment between the farthest entry point and the nearest exit point
      if (t1 > *hit_time) *hit_time = t1;
      if (t2 < t_max) t_max = t2;
      // If we ever exit before entering another slab, we don't pass through the intersecting volume
      if (*hit_time > t_max) return 0;
    }
  }
  // The ray intersects all 3 slabs
  return true;
}
