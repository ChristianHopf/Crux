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

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_intersecting_aabbs_true);
  RUN_TEST(test_non_intersecting_aabbs_false);
  return UNITY_END();
}
