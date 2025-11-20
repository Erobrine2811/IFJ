#!/bin/bash

# --- Configuration ---
INTERPRETER_PATH="/pub/courses/ifj/ic25int/linux/ic25int"
IFJ25_BIN="./ifj25"
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
get_test_category() {
    local test_file=$1
    if [[ "$test_file" == *"simple.wren"* ]] || [[ "$test_file" == *"frame.wren"* ]] || [[ "$test_file" == *"ex4.wren"* ]]; then
        echo "simple"
    else
        echo "advanced"
    fi
}

run_test() {
    local test_name=$1
    local test_dir="$TESTS_DIR/$test_name"
    local source_file="$test_dir/source.wren"
    local expected_output_file="$test_dir/expected_output"
    local actual_output_file="$TEMP_DIR/$test_name.actual_output"
    local compiled_code_file="$test_dir/compiled.out"
    local input_file="$test_dir/input.in"
    local category_file="$test_dir/category.txt"
    local test_category=$(cat "$category_file")

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

# --- Main execution ---
mkdir -p "$TEMP_DIR"

echo -e "${YELLOW}--- SKIPPING BUILD ---${NC}"
echo -e "${YELLOW}Please make sure you have built the project with 'make clean && make' before running this script.${NC}"

generate_tests

TEST_DIRS=$(find "$TESTS_DIR" -mindepth 1 -maxdepth 1 -type d -print0 | xargs -0 -n 1 basename | sort)

if [ -z "$TEST_DIRS" ]; then
    echo -e "${YELLOW}No test directories found in $TESTS_DIR.${NC}"
    exit 0
fi

for test_dir in $TEST_DIRS; do
    run_test "$test_dir"
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
