#!/usr/bin/env bash

GREEN="\033[1;32m"
BLUE="\033[1;34m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
DEFAULT="\033[0m"

test_prog="./ft_ssl"

normalize() {
	sed -E 's/^[^:]+: //'
}

echo -e "\n${YELLOW}--- md5 tests (refactor) ---${DEFAULT}\n"

total=0
passed=0
failed=0

# run_test "label" "expected_cmd" "actual_cmd"
run_test() {
    label="$1"
    expected_cmd="$2"
    actual_cmd="$3"
    ((total++))

    expected=$(eval "$expected_cmd" 2>&1)
    actual=$(eval "$actual_cmd" 2>&1)

    exp_norm=$(printf '%s' "$expected" | normalize | sed 's/[[:space:]]*$//')
    act_norm=$(printf '%s' "$actual" | normalize | sed 's/[[:space:]]*$//')

    if [ "$exp_norm" = "$act_norm" ]; then
        ((passed++))
        printf "\r[${GREEN}OK${DEFAULT}] %s\n" "$label"
    else
        ((failed++))
        printf "\r[${RED}FAIL${DEFAULT}] %s\n" "$label"
        echo "--- expected vs actual (normalized) ---"
        diff -u <(printf '%s' "$exp_norm") <(printf '%s' "$act_norm") || true
        echo
    fi
}

# helper to run the same template replacing md5 with $test_prog md5 for actual
run_template() {
    label="$1"
    template="$2"
    # expected uses system md5, actual uses $test_prog md5
    expected_cmd="$template"
    # replace only first occurrence of 'md5' with "$test_prog md5"
    actual_cmd="${template/ md5/ $test_prog md5}"
    actual_cmd="${actual_cmd/#md5/$test_prog md5}"
    run_test "$label" "$expected_cmd" "$actual_cmd"
}

# Tests: simple and file-based
run_template "echo pipe simple" "echo 'Hello, World!' | md5"
run_template "file Makefile" "md5 ./Makefile"
run_template "two files" "md5 ./Makefile ./README.md"
run_template "missing file" "md5 ./Makefile ./does_not_exist"

# -p (pipe) and -q
run_template "-p pipe echo" "echo 'Hello, World!' | md5 -p"
run_template "-q pipe echo" "echo 'Hello, World!' | md5 -q"
run_template "-q file" "md5 -q ./Makefile"
run_template "-q file + missing" "md5 -q ./Makefile ./does_not_exist"

# mix -p -r -s -q variations
run_template "-s simple" "md5 -s 'abc'"
run_template "-r with -s" "md5 -r -s 'abc'"
run_template "-q with -s" "md5 -q -s 'abc'"
run_template "-p then -s (pipe + string)" "md5 -p -s 'abc'"

# combined flags: -p -r
run_template "-p -r combo" "echo 'The quick brown fox' | md5 -p -r"
# -p and files together
run_template "-p and file" "echo 'mix' | md5 -p ./Makefile"

# more mixes: multiple -s and files
run_template "-s and file" "md5 -s 'mixstr' ./Makefile"

# Additional exhaustive variations
run_template "order -s then -q" "md5 -s 'abc' -q"
run_template "order -q then -s" "md5 -q -s 'abc'"

# combinations with files and flags
run_template "-r file" "md5 -r ./Makefile"
run_template "-q -r file" "md5 -q -r ./Makefile"
run_template "-p file (stdin+file)" "echo 'stdin' | md5 -p ./Makefile"
run_template "-p -s and file" "md5 -p -s 'mix' ./Makefile"

# multiple files with flags
run_template "-q multiple files" "md5 -q ./Makefile ./README.md"
run_template "-r multiple files" "md5 -r ./Makefile ./README.md"

# missing file + flags
run_template "-q missing file" "md5 -q ./does_not_exist"
run_template "-r missing file" "md5 -r ./does_not_exist"

echo -e "\n${YELLOW}Summary:${DEFAULT} Total=$total Passed=$passed Failed=$failed\n"

if [ $failed -gt 0 ]; then
    exit 1
else
    exit 0
fi