#pragma once

namespace gky {
struct test_manager {
  using test_fn = void (*)();

  int add_test(test_fn f)
  {
    all_tests[test_idx++] = f;
    return 0;
  }

  void run_tests()
  {
    for (auto t : all_tests)
      if (t)
        t();
  }

  int test_idx = 0;
  test_fn all_tests[100];
};

}  // namespace gky

extern gky::test_manager tm;

#define CAT_(x, y) x##y
#define CAT(x, y) CAT_(x, y)
#define unittest(FUN)           \
  void FUN();                   \
  auto CAT(FUN, __LINE__) = tm.add_test(FUN); \
  void FUN()

