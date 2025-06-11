#include "unity.h"
#include "physics/aabb.h"

void setUp() {
}

void tearDown() {
}

void test_intersecting_aabbs_true(void){
  struct AABB a = {
    .min = {1.0f, 1.0f, 1.0f},
    .max = {2.0f, 2.0f, 2.0f}
  };
  struct AABB b = {
    .min = {1.5f, 1.5f, 1.5f},
    .max = {2.5f, 2.5f, 2.5f}
  };

  TEST_ASSERT_TRUE(AABB_intersect(&a, &b));
}

void test_non_intersecting_aabbs_false(void){
  struct AABB a = {
    .min = {1.0f, 1.0f, 1.0f},
    .max = {2.0f, 2.0f, 2.0f}
  };
  struct AABB b = {
    .min = {3.0f, 3.0f, 3.0f},
    .max = {4.0f, 4.0f, 4.0f}
  };

  TEST_ASSERT_FALSE(AABB_intersect(&a, &b));
}

void test_aabb_update(void){
  struct AABB src = {
    .min = {-1.0f, -1.0f, -1.0f},
    .max = { 1.0f,  1.0f,  1.0f}
  };
  struct AABB dest = {
    .min = {0.0f, 0.0f, 0.0f},
    .max = {0.0f, 0.0f, 0.0f}
  };

  mat3 rotation;
  vec3 translation = {5.0f, 0.0f, 0.0f};

  glm_mat3_identity(rotation);
  glm_rotate_y(rotation, glm_rad(90.0f), rotation);

  AABB_update(&src, rotation, translation, &dest);

  struct AABB expected = {
    .min = {4.0f, -1.0f, -1.0f},
    .max = {6.0f, 1.0f, 1.0f}
  };
  assert_aabb_equal(&expected, &dest);
}

// Helpersa
void assert_vec3_equal(vec3 expected, vec3 actual, const char* message) {
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.0001f, expected[i], actual[i], message);
  }
}
void assert_aabb_equal(struct AABB *expected, struct AABB *actual){
  assert_vec3_equal(expected->min, actual->min, "AABB min is incorrect");
  assert_vec3_equal(expected->max, actual->max, "AABB max is incorrect");
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_intersecting_aabbs_true);
  RUN_TEST(test_non_intersecting_aabbs_false);
  RUN_TEST(test_aabb_update);
  return UNITY_END();
}
