#!/bin/bash

# --- Configuration ---
INTERPRETER_PATH="/pub/courses/ifj/ic25int/linux/ic25int"
IFJ25_BIN="./ifj25"
SIMPLE_TESTS_DIR="tests/simple/"
ADVANCED_TESTS_DIR="tests/advanced/"
BONUS_TESTS_DIR="tests/bonus/"

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
    local source_wren_file=$1
    local test_category=$2
    local test_base_dir=$(dirname "$source_wren_file")
    local test_name=$(basename "$test_base_dir")

    echo -e "${BLUE}--- Processing test: $test_name ($test_category) ---${NC}"

    local compiled_code_file_for_test="$TEMP_DIR/$test_name.compiled.out"
    local generated_ifj25code_target="$test_base_dir/generated.ifj25code"

    mkdir -p "$(dirname "$compiled_code_file_for_test")" # Ensure temp dir for compiled code exists

    # Compile source.wren once for the entire test case
    if ! "$IFJ25_BIN" "$source_wren_file" > "$compiled_code_file_for_test" 2>&1; then
        echo -e "${RED}FAIL: $test_name - ifj25 compilation failed.${NC}"
        cat "$compiled_code_file_for_test"
        # Save compilation error to generated.ifj25code
        cp "$compiled_code_file_for_test" "$generated_ifj25code_target"
        echo -e "${BLUE}Compilation error saved to: $generated_ifj25code_target${NC}"

        TEST_RESULTS+=("COMPILATION_FAIL")
        TEST_NAMES+=("$test_name")
        TEST_CATEGORIES+=("$test_category")
        return # Exit early if compilation fails, skipping all variations
    fi

    # Save the generated code to the test directory (only if compilation was successful)
    cp "$compiled_code_file_for_test" "$generated_ifj25code_target"
    echo -e "${BLUE}Generated code saved to: $generated_ifj25code_target${NC}"


    local test_variations=()
    # Check for subdirectories containing expected_output.txt
    while IFS= read -r -d '' sub_dir; do
        if [ -f "$sub_dir/expected_output.txt" ]; then
            test_variations+=("$sub_dir")
        fi
    done < <(find "$test_base_dir" -mindepth 1 -maxdepth 1 -type d -print0)

    # If no subdirectories with expected_output.txt, check the base directory itself
    if [ ${#test_variations[@]} -eq 0 ]; then
        if [ -f "$test_base_dir/expected_output.txt" ]; then
            test_variations+=("$test_base_dir")
        else
            echo -e "${YELLOW}Skipping $test_name: No expected_output.txt found in any variation.${NC}"
            TEST_RESULTS+=("SKIPPED")
            TEST_NAMES+=("$test_name")
            TEST_CATEGORIES+=("$test_category")
            return
        fi
    fi

    for variation_dir in "${test_variations[@]}"; do
        local variation_name=$(basename "$variation_dir")
        local current_test_name="$test_name"
        if [ "$variation_dir" != "$test_base_dir" ]; then
            current_test_name="$test_name/$variation_name"
        fi

        local expected_output_file="$variation_dir/expected_output.txt"
        local actual_output_file="$TEMP_DIR/$current_test_name.actual_output"
        local input_file="$variation_dir/source.in"

        echo -e "${BLUE}--- Running variation: $current_test_name ---${NC}"

        mkdir -p "$(dirname "$actual_output_file")"
        if [ -f "$input_file" ]; then
            if ! cat "$input_file" | "$INTERPRETER_PATH" "$compiled_code_file_for_test" > "$actual_output_file" 2>&1; then
                echo -e "${RED}FAIL: $current_test_name - Interpreter execution failed.${NC}"
                cat "$actual_output_file"
                TEST_RESULTS+=("FAIL")
                TEST_NAMES+=("$current_test_name")
                TEST_CATEGORIES+=("$test_category")
                continue # Move to next variation
            fi
        else
            if ! "$INTERPRETER_PATH" "$compiled_code_file_for_test" > "$actual_output_file" 2>&1; then
                echo -e "${RED}FAIL: $current_test_name - Interpreter execution failed.${NC}"
                cat "$actual_output_file"
                TEST_RESULTS+=("FAIL")
                TEST_NAMES+=("$current_test_name")
                TEST_CATEGORIES+=("$test_category")
                continue # Move to next variation
            fi
        fi

        # Filter actual output (remove first line for prompts if actual output has more than one line)
        local lines_in_actual_output=$(wc -l < "$actual_output_file")
        local file_to_compare="$actual_output_file"

        if [ "$lines_in_actual_output" -gt 1 ]; then
            local filtered_actual_output_file="$TEMP_DIR/$current_test_name.filtered_actual_output"
            tail -n +2 "$actual_output_file" > "$filtered_actual_output_file"
            file_to_compare="$filtered_actual_output_file"
        fi

        if diff -u "$expected_output_file" "$file_to_compare" > /dev/null; then
            echo -e "${GREEN}PASS: $current_test_name${NC}"
            TEST_RESULTS+=("PASS")
        else
            echo -e "${RED}FAIL: $current_test_name - Output mismatch.${NC}"
            diff -u "$expected_output_file" "$file_to_compare"
            TEST_RESULTS+=("FAIL")
        fi
        TEST_NAMES+=("$current_test_name")
        TEST_CATEGORIES+=("$test_category")
    done
}

TEMP_DIR=$(mktemp -d)

# --- Run Simple Tests ---
echo -e "${BLUE}--- Running Simple Tests ---${NC}"
while IFS= read -r -d '' source_wren_file; do
    run_test "$source_wren_file" "simple"
done < <(find "$SIMPLE_TESTS_DIR" -name "source.wren" -print0 | sort -z)

# --- Run Advanced Tests ---
echo -e "${BLUE}--- Running Advanced Tests ---${NC}"
while IFS= read -r -d '' source_wren_file; do
    run_test "$source_wren_file" "advanced"
done < <(find "$ADVANCED_TESTS_DIR" -name "source.wren" -print0 | sort -z)

# --- Run Bonus Tests ---
echo -e "${BLUE}--- Running Bonus Tests ---${NC}"
while IFS= read -r -d '' source_wren_file; do
    run_test "$source_wren_file" "bonus"
done < <(find "$BONUS_TESTS_DIR" -name "source.wren" -print0 | sort -z)

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
    elif [ "$result" == "COMPILATION_FAIL" ]; then
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
