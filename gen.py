import random
import string
import sys
import os

def generate_text_file(file_path, file_size_MB):
    # Convert megabytes to bytes
    file_size_bytes = file_size_MB * 1024 * 1024

    with open(file_path, 'w', encoding='ascii', errors='ignore') as file:
        # Generate random ASCII characters
        content = ''.join(random.choice(string.ascii_letters + string.digits + " " + "\n") for _ in range(file_size_bytes))

        # Write content to the file
        file.write(content)

if __name__ == "__main__":
    # Check if the correct number of command-line arguments is provided
    if len(sys.argv) != 2:
        print("Usage: python gen.py <file_size>")
        sys.exit(1)

    # Parse command-line arguments
    file_size_MB = int(sys.argv[1])
    output_directory = "test_cases"
    output_file_name = f"test_{file_size_MB}.txt"

    # Create the output directory if it doesn't exist
    os.makedirs(output_directory, exist_ok=True)

    # Construct the file path
    file_path = os.path.join(output_directory, output_file_name)

    # Generate the text file
    generate_text_file(file_path, file_size_MB)

    print(f"Successfully generated {output_file_name}")

