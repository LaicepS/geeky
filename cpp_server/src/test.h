#pragma once

namespace gky {

struct tester {
  using test_fn = void (*)();

  static tester& instance()
  {
    static tester instance;
    return instance;
  }

  auto add_test(test_fn f)
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

#define CAT_(x, y) x##y
#define CAT(x, y) CAT_(x, y)
#define unittest(FUN)           \
  void FUN();                   \
  auto CAT(FUN, __LINE__) = tester::instance().add_test(FUN); \
  void FUN()
