#!/bin/bash

# --- Configuration ---
INTERPRETER_PATH="wren_cli" # Assuming wren_cli is in the PATH
IFJ25_BIN="./ifj25"
EXAMPLES_DIR="examples"
TESTS_DIR="tests/generated"
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

generate_tests() {
    echo -e "${BLUE}--- Generating tests from examples ---${NC}"
    
    local i=0
    for example_file in $(find "$EXAMPLES_DIR" -name "*.wren" | sort); do
        local example_name=$(basename "$example_file" .wren)
        local test_name=$(printf "%02d_%s" "$i" "$example_name")
        local test_dir="$TESTS_DIR/$test_name"
        local source_file="$test_dir/source.wren"
        local expected_output_file="$test_dir/expected_output"
        local ifj25_output_file="$TEMP_DIR/${test_name}.ifj25_output"
        local input_file_src="$TESTS_DIR/$(printf "%02d_%s" "$i" "$example_name").in"
        local input_file_dest="$test_dir/input.in"

        mkdir -p "$test_dir"
        cp "$example_file" "$source_file"

        if [ -f "$input_file_src" ]; then
            cp "$input_file_src" "$input_file_dest"
        fi

        echo "Generating for $test_name"

        if ! "$IFJ25_BIN" "$source_file" > "$ifj25_output_file" 2>&1; then
            echo -e "${RED}FAIL (Generation): $test_name - ifj25 compilation failed.${NC}"
            cat "$ifj25_output_file"
            continue
        fi

        if [ -f "$input_file_dest" ]; then
            if ! cat "$input_file_dest" | "$INTERPRETER_PATH" "$ifj25_output_file" > "$expected_output_file" 2>&1; then
                echo -e "${RED}FAIL (Generation): $test_name - Interpreter execution failed.${NC}"
            fi
        else
            if ! "$INTERPRETER_PATH" "$ifj25_output_file" > "$expected_output_file" 2>&1; then
                echo -e "${RED}FAIL (Generation): $test_name - Interpreter execution failed.${NC}"
            fi
        fi
        i=$((i+1))
    done
    echo -e "${GREEN}--- Test generation complete ---${NC}"
}

run_test() {
    local test_name=$1
    local test_dir="$TESTS_DIR/$test_name"
    local source_file="$test_dir/source.wren"
    local expected_output_file="$test_dir/expected_output"
    local actual_output_file="$TEMP_DIR/$test_name.actual_output"
    local ifj25_output_file="$TEMP_DIR/$test_name.ifj25_output"
    local input_file="$test_dir/input.in"

    echo -e "${BLUE}--- Running test: $test_name ---${NC}"

    if [ ! -f "$expected_output_file" ]; then
        echo -e "${YELLOW}Skipping $test_name: expected_output not found.${NC}"
        TEST_RESULTS+=("SKIPPED")
        TEST_NAMES+=("$test_name")
        return
    fi

    if ! "$IFJ25_BIN" "$source_file" > "$ifj25_output_file" 2>&1; then
        echo -e "${RED}FAIL: $test_name - ifj25 compilation failed.${NC}"
        cat "$ifj25_output_file"
        TEST_RESULTS+=("FAIL")
        TEST_NAMES+=("$test_name")
        return
    fi

    if [ -f "$input_file" ]; then
        if ! cat "$input_file" | "$INTERPRETER_PATH" "$ifj25_output_file" > "$actual_output_file" 2>&1; then
            echo -e "${RED}FAIL: $test_name - Interpreter execution failed.${NC}"
            cat "$actual_output_file"
            TEST_RESULTS+=("FAIL")
            TEST_NAMES+=("$test_name")
            return
        fi
    else
        if ! "$INTERPRETER_PATH" "$ifj25_output_file" > "$actual_output_file" 2>&1; then
            echo -e "${RED}FAIL: $test_name - Interpreter execution failed.${NC}"
            cat "$actual_output_file"
            TEST_RESULTS+=("FAIL")
            TEST_NAMES+=("$test_name")
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
}

# --- Main execution ---
mkdir -p "$TEMP_DIR"

echo -e "${BLUE}--- SKIPPING BUILD ---${NC}"

if [ ! -x "$IFJ25_BIN" ]; then
    echo -e "${RED}ERROR: Compiler executable '$IFJ25_BIN' not found or not executable. Aborting tests.${NC}"
    exit 1
fi

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
echo "----------------------------------------------------------------"
printf "%-40s | %s\n" "TEST NAME" "RESULT"
echo "----------------------------------------------------------------"
for i in "${!TEST_NAMES[@]}"; do
    result="${TEST_RESULTS[$i]}"
    if [ "$result" == "PASS" ]; then
        printf "%-40s | ${GREEN}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    elif [ "$result" == "FAIL" ]; then
        printf "%-40s | ${RED}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    elif [ "$result" == "SKIPPED" ]; then
        printf "%-40s | ${YELLOW}%s${NC}\n" "${TEST_NAMES[$i]}" "$result"
    fi
done
echo "----------------------------------------------------------------"

# Clean up temporary files
rm -rf "$TEMP_DIR"

if [[ " ${TEST_RESULTS[@]} " =~ " FAIL " ]]; then
    exit 1
else
    exit 0
fi
