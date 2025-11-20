#!/bin/bash

# --- Configuration ---
INTERPRETER_PATH="/pub/courses/ifj/ic25int/linux/ic25int"
IFJ25_BIN="/Users/erobrine/Documents/school/3/ifj/ifj25"
SIMPLE_TESTS_DIR="tests/simple/"
ADVANCED_TESTS_DIR="tests/advanced/"

# --- Colors for output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# --- Test Results Array ---
declare -a TEST_RESULTS
declare -a TEST_NAMES
declare -a TEST_CATEGORIES

# --- Functions ---


run_test() {
    local test_dir_path=$1
    local test_category=$2
    local test_name=$(basename "$test_dir_path")
    local source_file="$(dirname "$test_dir_path")/source.wren"
    local expected_output_file="$test_dir_path/expected_output.txt"
    local actual_output_file="$TEMP_DIR/$test_name.actual_output"
    local compiled_code_file="$TEMP_DIR/$test_name.compiled.out"
    local input_file="$test_dir_path/source.in"

    echo -e "${BLUE}--- Running test: $test_name ($test_category) ---${NC}"

    if [ ! -f "$expected_output_file" ]; then
        echo -e "${YELLOW}Skipping $test_name: expected_output not found.${NC}"
        TEST_RESULTS+=("SKIPPED")
        TEST_NAMES+=("$test_name")
        TEST_CATEGORIES+=("$test_category")
        return
    fi


    if ! "$IFJ25_BIN" "$source_file" > "$compiled_code_file" 2>&1; then
        echo -e "${RED}FAIL: $test_name - ifj25 compilation failed.${NC}"
        cat "$compiled_code_file"
        TEST_RESULTS+=("FAIL")
        TEST_NAMES+=("$test_name")
        TEST_CATEGORIES+=("$test_category")
        return
    fi

    if [ -f "$input_file" ]; then
        if ! cat "$input_file" | "$INTERPRETER_PATH" "$compiled_code_file" > "$actual_output_file" 2>&1; then
            echo -e "${RED}FAIL: $test_name - Interpreter execution failed.${NC}"
            cat "$actual_output_file"
            TEST_RESULTS+=("FAIL")
            TEST_NAMES+=("$test_name")
            TEST_CATEGORIES+=("$test_category")
            return
        fi
    else
        if ! "$INTERPRETER_PATH" "$compiled_code_file" > "$actual_output_file" 2>&1; then
            echo -e "${RED}FAIL: $test_name - Interpreter execution failed.${NC}"
            cat "$actual_output_file"
            TEST_RESULTS+=("FAIL")
            TEST_NAMES+=("$test_name")
            TEST_CATEGORIES+=("$test_category")
            return
        fi
    fi

    if diff -u "$expected_output_file" "$actual_output_file" > /dev/null; then
        echo -e "${GREEN}PASS: $test_name${NC}"
        TEST_RESULTS+=("PASS")
    else
        echo -e "${RED}FAIL: $test_name - Output mismatch.${NC}"
        diff -u "$expected_output_file" "$actual_output_file"
        TEST_RESULTS+=("FAIL")
    fi
    TEST_NAMES+=("$test_name")
    TEST_CATEGORIES+=("$test_category")
}

TEMP_DIR=$(mktemp -d)

# --- Run Simple Tests ---
echo -e "${BLUE}--- Running Simple Tests ---${NC}"
find "$SIMPLE_TESTS_DIR" -name "expected_output.txt" -print0 | xargs -0 -n 1 dirname | sort | while read -r test_dir_path; do
    run_test "$test_dir_path" "simple"
done

# --- Run Advanced Tests ---
echo -e "${BLUE}--- Running Advanced Tests ---${NC}"
find "$ADVANCED_TESTS_DIR" -name "expected_output.txt" -print0 | xargs -0 -n 1 dirname | sort | while read -r test_dir_path; do
    run_test "$test_dir_path" "advanced"
done

echo -e "${BLUE}--- Test Summary ---${NC}"
echo "--------------------------------------------------------------------------------"
printf "%-40s | %-10s | %s\n" "TEST NAME" "CATEGORY" "RESULT"
echo "--------------------------------------------------------------------------------"
for i in "${!TEST_NAMES[@]}"; do
    result="${TEST_RESULTS[$i]}"
    category="${TEST_CATEGORIES[$i]}"
    if [ "$result" == "PASS" ]; then
        printf "%-40s | %-10s | ${GREEN}%s${NC}\n" "${TEST_NAMES[$i]}" "$category" "$result"
    elif [ "$result" == "FAIL" ]; then
        printf "%-40s | %-10s | ${RED}%s${NC}\n" "${TEST_NAMES[$i]}" "$category" "$result"
    elif [ "$result" == "SKIPPED" ]; then
        printf "%-40s | %-10s | ${YELLOW}%s${NC}\n" "${TEST_NAMES[$i]}" "$category" "$result"
    fi
done
echo "--------------------------------------------------------------------------------"

# Clean up temporary files
rm -rf "$TEMP_DIR"

if [[ " ${TEST_RESULTS[@]} " =~ " FAIL " ]]; then
    exit 1
else
    exit 0
fi
