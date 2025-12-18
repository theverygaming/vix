#include <vix/initfn.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/test.h>

static int test_variable = 0;
bool test_successful = true;

TEST(initfn_test) {
    return (test_variable == 12) && test_successful;
}

#define INITFN_TEST(n)                                      \
    static void initfn_test_##n() {                         \
        if (test_variable != n - 1) {                       \
            test_successful = false;                        \
        }                                                   \
        test_variable = n;                                  \
        kprintf(KP_INFO, "initfn test " STRINGIFY(n) "\n"); \
    }

INITFN_TEST(4)
INITFN_TEST(2)
INITFN_TEST(1)
INITFN_TEST(3)
INITFN_DEFINE(initfn_test_1, INITFN_PRE_MM_INIT, 0, initfn_test_1);
INITFN_DEFINE(
    initfn_test_2, INITFN_PRE_MM_INIT, 0, initfn_test_2, initfn_test_1
);
INITFN_DEFINE(
    initfn_test_3, INITFN_PRE_MM_INIT, 0, initfn_test_3, initfn_test_2
);
INITFN_DEFINE(
    initfn_test_4,
    INITFN_PRE_MM_INIT,
    0,
    initfn_test_4,
    initfn_test_1,
    initfn_test_3
);

INITFN_TEST(8)
INITFN_TEST(6)
INITFN_TEST(5)
INITFN_TEST(7)
INITFN_TEST(12)
INITFN_TEST(10)
INITFN_TEST(9)
INITFN_TEST(11)
INITFN_DEFINE(initfn_test_5, INITFN_EARLY_DRIVER_INIT, 0, initfn_test_5);
INITFN_DEFINE(
    initfn_test_6, INITFN_EARLY_DRIVER_INIT, 0, initfn_test_6, initfn_test_5
);
INITFN_DEFINE(
    initfn_test_7, INITFN_EARLY_DRIVER_INIT, 0, initfn_test_7, initfn_test_6
);
INITFN_DEFINE(
    initfn_test_8, INITFN_EARLY_DRIVER_INIT, 0, initfn_test_8, initfn_test_7
);
INITFN_DEFINE(
    initfn_test_9,
    INITFN_EARLY_DRIVER_INIT,
    0,
    initfn_test_9,
    initfn_test_5,
    initfn_test_8
);
INITFN_DEFINE(
    initfn_test_10,
    INITFN_EARLY_DRIVER_INIT,
    0,
    initfn_test_10,
    initfn_test_8,
    initfn_test_9
);
INITFN_DEFINE(
    initfn_test_11, INITFN_EARLY_DRIVER_INIT, 0, initfn_test_11, initfn_test_10
);
INITFN_DEFINE(
    initfn_test_12, INITFN_EARLY_DRIVER_INIT, 0, initfn_test_12, initfn_test_11
);
