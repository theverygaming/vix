import sys
import operator


class teststart_t:
    name = ""


class testsection_t:
    name = ""
    sectionname = ""
    success = False
    time = 0


class testresult_t:
    name = ""
    success = False
    time = 0


expected_tests = []

for line in sys.stdin:
    line = line.rstrip()
    split = line.split()
    symname = split[2]
    if symname.startswith("__test_func_"):
        start = teststart_t()
        start.name = symname[len("__test_func_"):]
        expected_tests.append(start)

if len(sys.argv) < 2:
    print(
        f"usage: {sys.argv[0]} filename\n-f : return fail exit code if a test failed")
    exit(1)

fail_on_failed_test = False

if len(sys.argv) == 3 and sys.argv[2] == "-f":
    fail_on_failed_test = True

teststarts = []
testsections = []
testresults = []

with open(sys.argv[1]) as file:
    for line in file:
        line = line.rstrip()
        split = line.split("|")
        if line.startswith("--TESTSTART--"):
            start = teststart_t()
            start.name = split[1]
            teststarts.append(start)
        if line.startswith("--TESTSECTION--"):
            section = testsection_t()
            section.success = split[1] == "G"
            section.name = split[2]
            section.sectionname = split[3]
            section.time = int(split[4])
            testsections.append(section)
        if line.startswith("--TESTRESULT--"):
            result = testresult_t()
            result.success = split[1] == "G"
            result.name = split[2]
            result.time = int(split[3])
            testresults.append(result)


def is_in_tarr(name, arr):
    for r in arr:
        if r.name == name:
            return True
    return False


for t in expected_tests:
    if not is_in_tarr(t.name, testresults):
        result = testresult_t()
        result.success = False
        result.name = t.name
        result.time = 0
        testresults.append(result)

for t in teststarts:
    if not is_in_tarr(t.name, testresults):
        result = testresult_t()
        result.success = False
        result.name = t.name
        result.time = 0
        testresults.append(result)

for t in testsections:
    if not is_in_tarr(t.name, testresults):
        result = testresult_t()
        result.success = False
        result.name = t.name
        result.time = 0
        testresults.append(result)

if not fail_on_failed_test:
    print("<details>\n<summary>qemu output</summary>\n\n```")
    with open(sys.argv[1]) as file:
        print(file.read())
    print("```\n\n</details>\n")

if len(testresults) == 0:
    exit(0)

failed = 0
for test in testresults:
    if not test.success:
        failed += 1

if not fail_on_failed_test:
    emoji = ":heavy_check_mark:"
    if failed != 0:
        emoji = ":x:"
    print(f"# Tests {emoji}")
    print(f"**{len(testresults)}** tests completed with **{len(testresults)-failed}** passed and **{failed}** failed")
    print("|name|passed?|total time (ms)|")
    print("|:--:|:-----:|:-------------:|")

for test in testresults:
    if fail_on_failed_test and not test.success:
        exit(1)
    if not fail_on_failed_test:
        pass_str = ":heavy_check_mark:" if test.success else ":x:"
        print(f"{test.name}|{pass_str}|{test.time}")

if fail_on_failed_test:
    exit(0)

if len(testsections) != 0:
    print(f"## test sections")
    print("|name|section name|passed?|time (ms)|")
    print("|:--:|:----------:|:-----:|:-------:|")

for t in testresults:
    if is_in_tarr(t.name, testsections):
        tpass_str = ":heavy_check_mark:" if t.success else ":x:"
        for s in testsections:
            if s.name == t.name:
                pass_str = ":heavy_check_mark:" if s.success else ":x:"
                print(f"{t.name} {tpass_str}|{s.sectionname}|{pass_str}|{s.time}")
