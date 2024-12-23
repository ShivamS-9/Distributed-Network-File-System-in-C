#!/bin/bash

# Create test_cases directory if it doesn't exist
mkdir -p test_cases

# Array of file sizes
file_sizes=(15 30 100 300 500)

# Create paths.txt file
touch test_cases/paths.txt
echo test_cases >> test_cases/paths.txt

# Iterate over file sizes and generate files
for size in "${file_sizes[@]}"; do
    output_file="test_${size}.txt"
    python3 gen.py "$size"
    echo "test_cases/$output_file" >> test_cases/paths.txt
done

echo "Test cases generated in the test_cases directory."

# Check if the number of copies argument is provided
if [ "$#" -eq 1 ]; then
    source_dir="SS"
    num_copies=$1

    # Loop to create copies
    for ((i=1; i<=$num_copies; i++)); do
        # Copy the directory and its contents
        cp -r "${source_dir}" "${source_dir}_${i}"
    done

    echo "Generated ${num_copies} copies of ${source_dir}"
else
    echo "No argument provided. Only test cases directory created."
fi