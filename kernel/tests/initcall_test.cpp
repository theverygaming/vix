#include <vix/initcall.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/test.h>

static int test_variable = 0;
bool test_successful = true;

TEST(initcall_test) {
    return (test_variable == 12) && test_successful;
}

#define INITCALL_TEST(n)                                      \
    static int initcall_test_##n() {                          \
        if (test_variable != n - 1) {                         \
            test_successful = false;                          \
        }                                                     \
        test_variable = n;                                    \
        kprintf(KP_INFO, "initcall test " STRINGIFY(n) "\n"); \
        return 0;                                             \
    }

INITCALL_TEST(4)
INITCALL_TEST(2)
INITCALL_TEST(1)
INITCALL_TEST(3)
DEFINE_INITCALL(INITCALL_PRE_MM_INIT, 18, initcall_test_1);
DEFINE_INITCALL(INITCALL_PRE_MM_INIT, INITCALL_PRIO_DEFAULT, initcall_test_4);
DEFINE_INITCALL(INITCALL_PRE_MM_INIT, 69, initcall_test_2);
DEFINE_INITCALL(INITCALL_PRE_MM_INIT, 420, initcall_test_3);

INITCALL_TEST(8)
INITCALL_TEST(6)
INITCALL_TEST(5)
INITCALL_TEST(7)
INITCALL_TEST(12)
INITCALL_TEST(10)
INITCALL_TEST(9)
INITCALL_TEST(11)
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 0, initcall_test_5);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 1025, initcall_test_8);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 123, initcall_test_6);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, INITCALL_PRIO_DEFAULT, initcall_test_7);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 1999, initcall_test_9);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 1000000, initcall_test_12);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 2000, initcall_test_10);
DEFINE_INITCALL(INITCALL_EARLY_DRIVER_INIT, 99999, initcall_test_11);
