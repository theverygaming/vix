#include <config.h>
#include <stdio.h>
#include <test.h>
#include <time.h>

#if defined(CONFIG_ARCH_X86) || defined(CONFIG_ARCH_M68K)
#include <stdlib.h>
#include <symbols.h>
#endif

static const char *current_test_name = nullptr;
static uint64_t current_test_starttime;
static uint64_t current_section_starttime;
static uint64_t current_section_extratime;

void run_all_tests() {
#if defined(CONFIG_ARCH_X86) || defined(CONFIG_ARCH_M68K)
    const char *tfstr = "__test_func_";
    size_t tflen = strlen(tfstr);
    size_t n = 0;
    std::pair<const char *, uintptr_t> s = syms::get_sym(n++);
    while (s.first != nullptr) {
        if (strncmp(s.first, tfstr, tflen) == 0) {
            current_test_name = s.first + tflen;
            bool (*testfunc)() = (bool (*)())s.second;

            printf("--TESTSTART--|%s\n", current_test_name);
            current_section_extratime = 0;
            current_test_starttime = current_section_starttime = time::ns_since_bootup;
            bool result = testfunc();
            uint64_t ns2 = time::ns_since_bootup;
            unsigned int ns = ((ns2 - current_test_starttime) - current_section_extratime);

            if (result) {
                printf("--TESTRESULT--|G|%s|%u\n", current_test_name, ns);
            } else {
                printf("--TESTRESULT--|F|%s|%u\n", current_test_name, ns);
            }
            current_test_name = nullptr;
        }
        s = syms::get_sym(n++);
    }
#endif
}

void test::test_section(const char *name, bool status) {
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
}

TEST(tests_work) {
    return true;
}
