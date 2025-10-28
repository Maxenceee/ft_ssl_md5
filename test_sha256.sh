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

echo -e "\n${YELLOW}--- sha256 tests (refactor) ---${DEFAULT}\n"

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

# helper to run the same template replacing sha256 with $test_prog sha256 for actual
run_template() {
    label="$1"
    template="$2"
    # expected uses system sha256, actual uses $test_prog sha256
    expected_cmd="$template"
    # replace only first occurrence of 'sha256' with "$test_prog sha256"
    actual_cmd="${template/ sha256/ $test_prog sha256}"
    actual_cmd="${actual_cmd/#sha256/$test_prog sha256}"
    run_test "$label" "$expected_cmd" "$actual_cmd"
}

# Tests: simple and file-based
run_template "echo pipe simple" "echo 'Hello, World!' | sha256"
run_template "file Makefile" "sha256 ./Makefile"
run_template "two files" "sha256 ./Makefile ./README.md"
run_template "missing file" "sha256 ./Makefile ./does_not_exist"

# -p (pipe) and -q
run_template "-p pipe echo" "echo 'Hello, World!' | sha256 -p"
run_template "-q pipe echo" "echo 'Hello, World!' | sha256 -q"
run_template "-q file" "sha256 -q ./Makefile"
run_template "-q file + missing" "sha256 -q ./Makefile ./does_not_exist"

# mix -p -r -s -q variations
run_template "-s simple" "sha256 -s 'abc'"
run_template "-r with -s" "sha256 -r -s 'abc'"
run_template "-q with -s" "sha256 -q -s 'abc'"
run_template "-p then -s (pipe + string)" "sha256 -p -s 'abc'"

# combined flags: -p -r
run_template "-p -r combo" "echo 'The quick brown fox' | sha256 -p -r"
# -p and files together
run_template "-p and file" "echo 'mix' | sha256 -p ./Makefile"

# more mixes: multiple -s and files
run_template "-s and file" "sha256 -s 'mixstr' ./Makefile"

# Additional exhaustive variations
run_template "order -s then -q" "sha256 -s 'abc' -q"
run_template "order -q then -s" "sha256 -q -s 'abc'"

# combinations with files and flags
run_template "-r file" "sha256 -r ./Makefile"
run_template "-q -r file" "sha256 -q -r ./Makefile"
run_template "-p file (stdin+file)" "echo 'stdin' | sha256 -p ./Makefile"
run_template "-p -s and file" "sha256 -p -s 'mix' ./Makefile"

# multiple files with flags
run_template "-q multiple files" "sha256 -q ./Makefile ./README.md"
run_template "-r multiple files" "sha256 -r ./Makefile ./README.md"

# missing file + flags
run_template "-q missing file" "sha256 -q ./does_not_exist"
run_template "-r missing file" "sha256 -r ./does_not_exist"

echo -e "\n${YELLOW}Summary:${DEFAULT} Total=$total Passed=$passed Failed=$failed\n"

if [ $failed -gt 0 ]; then
    exit 1
else
    exit 0
fi