import csv
import glob
import os


def parse_file(filepath):
    """Parses a single text file and returns a dictionary of its data."""
    data = {}

    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.read().splitlines()

    if not lines:
        return data

    # The first line is the instance name (no colon)
    data["Instance"] = lines[0].strip()

    # Parse the rest of the lines
    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue

        if ":" in line:
            # Matches formats like "map: ../data..." or "CBS:6.26056"
            key, val = line.split(":", 1)
            data[key.strip()] = val.strip()

        elif line.endswith("failed."):
            # Matches "CBS failed." or "CBSH failed."
            key = line.split()[0]
            data[f"{key}_status"] = "failed"

        elif " took " in line:
            # Matches "BCP took 1083 ms" or "ALL took 579635 ms"
            parts = line.split(" took ")
            key = f"{parts[0].strip()}_took"
            data[key] = parts[1].strip()

        else:
            # Catch-all for any unexpected lines
            data["Unparsed_Line"] = line

    return data


def convert_directory_to_csv(input_dir, output_file):
    """Reads all txt files in a directory and writes them to a CSV."""
    all_rows = []
    all_columns = set()

    # 1. Read all files and collect data + column names
    search_pattern = os.path.join(input_dir, "*.txt")
    file_list = glob.glob(search_pattern)

    if not file_list:
        print(f"No .txt files found in {input_dir}")
        return

    for file in file_list:
        file_data = parse_file(file)
        if file_data:
            all_rows.append(file_data)
            all_columns.update(file_data.keys())

    # 2. Organize column order (Put 'Instance', 'map', 'scenario' first if they exist)
    ordered_columns = ["Instance"]
    for priority_col in ["map", "scenario"]:
        if priority_col in all_columns:
            ordered_columns.append(priority_col)

    # Add the remaining columns in alphabetical order
    remaining_cols = sorted([col for col in all_columns if col not in ordered_columns])
    ordered_columns.extend(remaining_cols)

    # 3. Write to CSV
    with open(output_file, "w", newline="", encoding="utf-8") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=ordered_columns)
        writer.writeheader()
        writer.writerows(all_rows)

    print(f"Successfully converted {len(all_rows)} files into {output_file}")


# ==========================================
# Configuration and Execution
# ==========================================
if __name__ == "__main__":
    # Change these paths to match your system
    INPUT_DIRECTORY = "./output"
    OUTPUT_CSV = "results_output.csv"

    convert_directory_to_csv(INPUT_DIRECTORY, OUTPUT_CSV)
