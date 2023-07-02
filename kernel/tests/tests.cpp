#include <config.h>
#include <test.h>

#ifdef CONFIG_ARCH_X86
#include <arch/symbols.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

static const char *current_test_name = nullptr;
static uint64_t current_test_starttime;
static uint64_t current_section_starttime;
static uint64_t current_section_extratime;

void run_all_tests() {
#ifdef CONFIG_ARCH_X86
    const char *tfstr = "__test_func_";
    size_t tflen = strlen(tfstr);
    size_t n = 0;
    std::pair<const char *, uintptr_t> s = syms::get_sym(n);
    while (s.first != nullptr) {
        if (strncmp(s.first, tfstr, tflen) == 0) {
            current_test_name = s.first + tflen;
            bool (*testfunc)() = (bool (*)())s.second;

            printf("--TESTSTART--|%s\n", current_test_name);
            current_section_extratime = 0;
            current_test_starttime = current_section_starttime = time::ms_since_bootup;
            bool result = testfunc();
            uint64_t ms2 = time::ms_since_bootup;
            unsigned int ms = ((ms2 - current_test_starttime) - current_section_extratime);

            if (result) {
                printf("--TESTRESULT--|G|%s|%u\n", current_test_name, ms);
            } else {
                printf("--TESTRESULT--|F|%s|%u\n", current_test_name, ms);
            }
            current_test_name = nullptr;
        }
        s = syms::get_sym(n++);
    }
#endif
}

void test::test_section(const char *name, bool status) {
    uint64_t ms2 = time::ms_since_bootup;
    if (current_test_name == nullptr) {
        return;
    }
    unsigned int ms = (ms2 - current_section_starttime);
    if (status) {
        printf("--TESTSECTION--|G|%s|%s|%u\n", current_test_name, name, ms);
    } else {
        printf("--TESTSECTION--|F|%s|%s|%u\n", current_test_name, name, ms);
    }
    current_section_starttime = time::ms_since_bootup;
    current_section_extratime += current_section_starttime - ms2;
}

TEST(tests_work) {
    return true;
}
