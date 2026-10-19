#include <vix/config.h>
#include <vix/stdio.h>
#include <vix/test.h>
#include <vix/time.h>

#ifdef CONFIG_ENABLE_TESTS
static const char *current_test_name = nullptr;
static uint64_t current_test_starttime;
static uint64_t current_section_starttime;
static uint64_t current_section_extratime;

extern "C" struct _test_def *START_TEST_DEFS;
extern "C" struct _test_def *END_TEST_DEFS;
#endif

void run_all_tests() {
#ifdef CONFIG_ENABLE_TESTS
    for (struct _test_def **tdi = &START_TEST_DEFS; tdi < &END_TEST_DEFS; tdi++) {
        struct _test_def *td = *tdi;
        current_test_name = td->name;
        printf("--TESTSTART--|%s\n", current_test_name);
        current_section_extratime = 0;
        current_test_starttime = current_section_starttime = time::ns_since_bootup;
        bool result = td->testfn();
        uint64_t ns2 = time::ns_since_bootup;
        unsigned int ns = ((ns2 - current_test_starttime) - current_section_extratime);

        if (result) {
            printf("--TESTRESULT--|G|%s|%u\n", current_test_name, ns);
        } else {
            printf("--TESTRESULT--|F|%s|%u\n", current_test_name, ns);
        }
        current_test_name = nullptr;
    }
#endif
}

void test::test_section(const char *name, bool status) {
#ifdef CONFIG_ENABLE_TESTS
    uint64_t ns2 = time::ns_since_bootup;
    if (current_test_name == nullptr) {
        return;
    }
    unsigned int ns = (ns2 - current_section_starttime);
    if (status) {
        printf("--TESTSECTION--|G|%s|%s|%u\n", current_test_name, name, ns);
    } else {
        printf("--TESTSECTION--|F|%s|%s|%u\n", current_test_name, name, ns);
    }
    current_section_starttime = time::ns_since_bootup;
    current_section_extratime += current_section_starttime - ns2;
#endif
}

TEST(tests_work) {
    return true;
}
