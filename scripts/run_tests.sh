#!/bin/bash

# --- Configuration ---
INTERPRETER_PATH="" # Placeholder: User will provide this later
IFJ25_BIN="./ifj25"
TESTS_DIR="tests"
TEMP_DIR="temp_test_output"

# --- Colors for output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# --- Test Results Array ---
declare -a TEST_RESULTS
declare -a TEST_NAMES

# --- Functions ---
run_test() {
    local test_name=$1
    local source_file="$TESTS_DIR/$test_name/source.wren"
    local expected_output_file="$TESTS_DIR/$test_name/expected_output"
    local actual_output_file="$TEMP_DIR/$test_name.actual_output"
    local ifj25_output_file="$TEMP_DIR/$test_name.ifj25_output"

    echo -e "${BLUE}--- Running test: $test_name ---${NC}"

    if [ ! -f "$source_file" ]; then
        echo -e "${YELLOW}Skipping $test_name: source.wren not found.${NC}"
        TEST_RESULTS+=("SKIPPED")
        TEST_NAMES+=("$test_name")
        return
    fi

    if [ ! -f "$expected_output_file" ]; then
        echo -e "${YELLOW}Skipping $test_name: expected_output not found.${NC}"
        TEST_RESULTS+=("SKIPPED")
        TEST_NAMES+=("$test_name")
        return
    fi

    # Run ifj25 and pipe to interpreter
    if ! "$IFJ25_BIN" "$source_file" > "$ifj25_output_file" 2>&1; then
        echo -e "${RED}FAIL: $test_name - ifj25 compilation failed.${NC}"
        echo -e "${RED}ifj25 output:${NC}"
        cat "$ifj25_output_file"
        TEST_RESULTS+=("FAIL")
        TEST_NAMES+=("$test_name")
        return
    fi

    if [ -z "$INTERPRETER_PATH" ]; then
        echo -e "${YELLOW}WARNING: INTERPRETER_PATH is not set. Cannot run interpreter for $test_name.${NC}"
        echo -e "${YELLOW}Consider this test as 'IFJ25_ONLY_PASS' if ifj25 compilation was successful.${NC}"
        TEST_RESULTS+=("IFJ25_ONLY_PASS")
        TEST_NAMES+=("$test_name")
        return
    fi

    if ! "$INTERPRETER_PATH" < "$ifj25_output_file" > "$actual_output_file" 2>&1; then
        echo -e "${RED}FAIL: $test_name - Interpreter execution failed.${NC}"
        echo -e "${RED}Interpreter output:${NC}"
        cat "$actual_output_file"
        TEST_RESULTS+=("FAIL")
        TEST_NAMES+=("$test_name")
        return
    fi

    # Compare outputs
    if diff -u "$expected_output_file" "$actual_output_file" > /dev/null; then
        echo -e "${GREEN}PASS: $test_name${NC}"
        TEST_RESULTS+=("PASS")
    else
        echo -e "${RED}FAIL: $test_name - Output mismatch.${NC}"
        echo -e "${RED}--- Diff for $test_name ---${NC}"
        diff -u "$expected_output_file" "$actual_output_file"
        echo -e "${RED}--------------------------${NC}"
        TEST_RESULTS+=("FAIL")
    fi
    TEST_NAMES+=("$test_name")
}

# --- Main execution ---
mkdir -p "$TEMP_DIR"

echo -e "${BLUE}--- Cleaning and building project ---${NC}"
if ! make clean && make; then
    echo -e "${RED}ERROR: Project compilation failed. Aborting tests.${NC}"
    exit 1
fi
echo -e "${GREEN}Project built successfully.${NC}"

# Find all test directories
TEST_DIRS=$(find "$TESTS_DIR" -mindepth 1 -maxdepth 1 -type d -print0 | xargs -0 -n 1 basename)

if [ -z "$TEST_DIRS" ]; then
    echo -e "${YELLOW}No test directories found in $TESTS_DIR. Please create test cases.${NC}"
    exit 0
fi

for test_dir in $TEST_DIRS; do
    run_test "$test_dir"
done

echo -e "${BLUE}--- Test Summary ---${NC}"
echo "----------------------------------------------------------------"
printf "% -30s | %s\n" "TEST NAME" "RESULT"
echo "----------------------------------------------------------------"
for i in "${!TEST_NAMES[@]}"; do
    result="${TEST_RESULTS[$i]}"
    if [ "$result" == "PASS" ]; then
        printf "% -30s | ${GREEN}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    elif [ "$result" == "FAIL" ]; then
        printf "% -30s | ${RED}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    elif [ "$result" == "SKIPPED" ]; then
        printf "% -30s | ${YELLOW}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    elif [ "$result" == "IFJ25_ONLY_PASS" ]; then
        printf "% -30s | ${YELLOW}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    fi
done
echo "----------------------------------------------------------------"

# Clean up temporary files
rm -rf "$TEMP_DIR"

# Exit with non-zero status if any test failed
if [[ " ${TEST_RESULTS[@]} " =~ " FAIL " ]]; then
    exit 1
else
    exit 0
fi
