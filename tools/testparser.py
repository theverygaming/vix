import sys
import operator

class test_t:
    result = ""
    group = ""
    name = ""
    file = ""
    line = ""
    condition = ""

if len(sys.argv) < 2:
    print(f"usage: {sys.argv[0]} filename\n-f : return fail exit code if a test failed")
    exit(1)

fail_on_failed_test = False

if len(sys.argv) == 3 and sys.argv[2] == "-f":
    fail_on_failed_test = True

tests = []

with open(sys.argv[1]) as file:
    for line in file:
        line = line.rstrip()
        if line.startswith("--TEST--"):
            test_p = line.split("|")
            test = test_t()
            test.result = test_p[1]
            test.group = test_p[2]
            test.name = test_p[3]
            test.file = test_p[4]
            test.line = test_p[5]
            test.condition = test_p[6]
            tests.append(test)

tests.sort(key=operator.attrgetter('group'))

failed = 0
for test in tests:
    if test.result == "F":
        failed += 1

if not fail_on_failed_test:
    emoji = ":heavy_check_mark:"
    if failed != 0:
        emoji = ":x:"
    print(f"# Tests {emoji}")
    print(f"**{len(tests)}** tests completed with **{len(tests)-failed}** passed and **{failed}** failed")
    print("|group|name|passed|failed|file|")
    print("|:---:|:---:|:---:|:---:|:---:|")

for test in tests:
    if fail_on_failed_test and test.result == "F":
        exit(1)
    if not fail_on_failed_test:
        pass_str = ":heavy_check_mark:|"
        if test.result == "F":
            pass_str = "|:x:"
        print(f"|{test.group}|{test.name}|{pass_str}|{test.file}:{test.line}")
