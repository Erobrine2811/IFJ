#!/bin/bash

# A script with aggressive debugging to diagnose the submission packaging.

set -e
set -x # Print each command before executing

echo "--- RUNNING THE DEBUGGING SUBMISSION SCRIPT ---"

# Clean the project first
echo "--- Cleaning project directory (make clean) ---"
make clean

# Define directories and files
SUBMISSION_DIR="completed_project"
ARCHIVE_FILE="xcapkoe00.zip"
VALIDATION_DIR="submission_test_dir"

# Create a clean submission directory
echo "--- Preparing submission directory: $SUBMISSION_DIR ---"
rm -rf "$SUBMISSION_DIR"
mkdir -p "$SUBMISSION_DIR"

# Copy source files explicitly
echo "--- Copying source files (.c and .h) into $SUBMISSION_DIR ---"
cp src/*.c "$SUBMISSION_DIR/"
cp src/*.h "$SUBMISSION_DIR/"

# Copy other required files
echo "--- Copying Makefile into $SUBMISSION_DIR ---"
cp Makefile "$SUBMISSION_DIR/"

# Modify the Makefile inside the submission directory to remove "src/" paths
# Using -i.bak for macOS compatibility
echo "--- Modifying Makefile inside $SUBMISSION_DIR for flat structure ---"
sed -i.bak 's|src/||g' "$SUBMISSION_DIR/Makefile"

# Copy optional files if they exist
if [ -f "dokumentace.pdf" ]; then
    echo "--- Copying dokumentace.pdf ---"
    cp "dokumentace.pdf" "$SUBMISSION_DIR/"
fi
if [ -f "rozdeleni" ]; then
    echo "--- Copying rozdeleni ---"
    cp "rozdeleni" "$SUBMISSION_DIR/"
fi
if [ -f "rozsireni" ]; then
    echo "--- Copying rozsireni ---"
    cp "rozsireni" "$SUBMISSION_DIR/"
fi

# Show final structure before zipping
echo "--- Final structure of '$SUBMISSION_DIR' before zipping: ---"
ls -lR "$SUBMISSION_DIR"
echo "------------------------------------------------------------"

# Create the archive from the flattened directory
echo "--- Creating zip archive: $ARCHIVE_FILE ---"
rm -f "$ARCHIVE_FILE"
cd "$SUBMISSION_DIR"
zip -r "../$ARCHIVE_FILE" .
cd ..
rm -rf "$SUBMISSION_DIR"

# Clean up and prepare for validation
echo "--- Preparing to run validation script ---"
rm -rf "$VALIDATION_DIR"
mkdir -p "$VALIDATION_DIR"

# Explicitly define and show the command before running
VALIDATION_COMMAND="./scripts/is_it_ok.sh $ARCHIVE_FILE $VALIDATION_DIR"
echo "--- EXECUTING VALIDATION: $VALIDATION_COMMAND ---"
./scripts/is_it_ok.sh "$ARCHIVE_FILE" "$VALIDATION_DIR"

echo "--- SCRIPT FINISHED ---"
set +x
