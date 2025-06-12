#include "unity.h"
#include "physics/aabb.h"
#include "physics/utils.h"

void setUp() {
}

void tearDown() {
}

// Helpers
void assert_vec3_equal(vec3 expected, vec3 actual, const char* message) {
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.0001f, expected[i], actual[i], message);
  }
}
void assert_aabb_equal(struct AABB *expected, struct AABB *actual){
  assert_vec3_equal(expected->center, actual->center, "AABB min is incorrect");
  assert_vec3_equal(expected->extents, actual->extents, "AABB max is incorrect");
}

// COLLISION TESTS
//
void test_intersecting_aabbs_true(void){
  struct AABB a = {
    .center = {1.5f, 1.5f, 1.5f},
    .extents = {0.5f, 0.5f, 0.5f}
    // .min = {1.0f, 1.0f, 1.0f},
    // .max = {2.0f, 2.0f, 2.0f}
  };
  struct AABB b = {
    .center = {2.0f, 2.0f, 2.0f},
    .extents = {0.5f, 0.5f, 0.5f}
    // .min = {1.5f, 1.5f, 1.5f},
    // .max = {2.5f, 2.5f, 2.5f}
  };

  TEST_ASSERT_TRUE(AABB_intersect_AABB(&a, &b));
}

void test_non_intersecting_aabbs_false(void){
  struct AABB a = {
    .center = {1.5f, 1.5f, 1.5f},
    .extents = {0.5f, 0.5f, 0.5f}
    // .min = {1.0f, 1.0f, 1.0f},
    // .max = {2.0f, 2.0f, 2.0f}
  };
  struct AABB b = {
    .center = {3.5f, 3.5f, 3.5f},
    .extents = {0.5f, 0.5f, 0.5f}
    // .min = {3.0f, 3.0f, 3.0f},
    // .max = {4.0f, 4.0f, 4.0f}
  };

  TEST_ASSERT_FALSE(AABB_intersect_AABB(&a, &b));
}

void test_aabb_intersect_plane_true(void){
  struct AABB box = {
    .center = {1.5f, 1.5f, 1.5f},
    .extents = {0.5f, 0.5f, 0.5f}
    // .min = {1.0f, 1.0f, 1.0f},
    // .max = {2.0f, 2.0f, 2.0f}
  };

  // Trivial case: plane is the xz plane
  struct PlaneCollider planeSimple = {
    .normal = {0.0f, 1.0f, 0.0f},
    .distance = 1.5f
  };
  // Plane is not xz, xy, or yz plane
  struct PlaneCollider plane2 = {
    .normal = {0.577f, 0.577f, 0.577f},
    .distance = 2.0f
  };
  print_aabb(&box);
  print_plane_collider(&plane2);

  TEST_ASSERT_TRUE(AABB_intersect_plane(&box, &planeSimple));
  TEST_ASSERT_TRUE(AABB_intersect_plane(&box, &plane2));
}

void test_aabb_intersect_plane_false(void){
  struct AABB box = {
    .center = {1.5f, 1.5f, 1.5f},
    .extents = {0.5f, 0.5f, 0.5f}
    // .min = {1.0f, 1.0f, 1.0f},
    // .max = {2.0f, 2.0f, 2.0f}
  };

  // Trivial case: plane is the xz plane
  struct PlaneCollider planeSimple = {
    .normal = {0.0f, 1.0f, 0.0f},
    .distance = 0.0f
  };
  // Plane is not xz, xy, or yz plane
  struct PlaneCollider plane2 = {
    .normal = {0.577f, 0.577f, 0.577f},
    .distance = 10.0f
  };
  

  TEST_ASSERT_FALSE(AABB_intersect_plane(&box, &planeSimple));
  TEST_ASSERT_FALSE(AABB_intersect_plane(&box, &plane2));
}


// OTHER TESTS
//
void test_aabb_update(void){
  struct AABB src = {
    .center = {0.0f, 0.0f, 0.0f},
    .extents = {1.0f, 1.0f, 1.0f}
    // .min = {-1.0f, -1.0f, -1.0f},
    // .max = { 1.0f,  1.0f,  1.0f}
  };
  struct AABB dest = {
    .center = {0.0f, 0.0f, 0.0f},
    .extents = {0.0f, 0.0f, 0.0f}
  };

  mat3 rotation;
  vec3 translation = {5.0f, 0.0f, 0.0f};

  mat4 rotation_mat4;
  glm_mat4_identity(rotation_mat4);
  glm_rotate_y(rotation_mat4, glm_rad(90.0f), rotation_mat4);
  glm_mat4_pick3(rotation_mat4, rotation);

  AABB_update(&src, rotation, translation, &dest);

  struct AABB expected = {
    .center = {5.0f, 0.0f, 0.0f},
    .extents = {1.0f, 1.0f, 1.0f}
    // .min = {4.0f, -1.0f, -1.0f},
    // .max = {6.0f, 1.0f, 1.0f}
  };
  assert_aabb_equal(&expected, &dest);
}


int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_intersecting_aabbs_true);
  RUN_TEST(test_non_intersecting_aabbs_false);
  RUN_TEST(test_aabb_intersect_plane_true);
  RUN_TEST(test_aabb_intersect_plane_false);
  RUN_TEST(test_aabb_update);
  return UNITY_END();
}
