import pandas as pd
import glob
import os


def generate_markdown_table(input_folder, output_file, column_order, table_name):
    # Define the path pattern for the CSV files relative to the script's directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_pattern = os.path.join(script_dir, f"../compression_ratio_result/{input_folder}/*.csv")
    csv_files = glob.glob(file_pattern)

    # Define a dictionary to rename files to match the specified column names
    file_to_column = {
        "gorillas": "Gor",
        "chimp": "Ch",
        "chimp128": "Ch128",
        "patas": "Patas",
        "pde": "PDE",
        "elf": "Elf",
        "alp": "Alp",
        "lwc_alp": "LWC+Alp",
        "zstd": "Zstd"
    }

    # Initialize an empty DataFrame to hold all datasets
    df_combined = pd.DataFrame()

    # Load data from each file and insert it into the correct column
    for file in csv_files:
        # Extract the prefix of the file to use as the column name
        prefix = file.split("/")[-1].split(".csv")[0].lower()
        if prefix in file_to_column:
            column_name = file_to_column[prefix]
            df = pd.read_csv(file)
            df = df.rename(columns={"size": column_name})
            df.set_index("dataset", inplace=True)

            # Combine this DataFrame with the main DataFrame
            df_combined = df_combined.combine_first(df) if not df_combined.empty else df

    # Reset the index and rename it to "Dataset" for the final table
    df_combined.reset_index(inplace=True)
    df_combined.rename(columns={"dataset": "Dataset"}, inplace=True)

    # Ensure all expected columns are included in the specified order
    for col in column_order:
        if col not in df_combined.columns:
            df_combined[col] = ""  # Add empty columns if missing

    # Reorder the columns to match the specified order
    df_combined = df_combined[column_order]

    # Fill NaN with an empty string for better readability in terminal
    df_combined = df_combined.fillna("")

    # Generate the console-friendly table with aligned columns
    col_widths = [max(len(str(value)) for value in df_combined[col].astype(str).tolist() + [col]) + 2 for col in
                  df_combined.columns]
    total_width = sum(col_widths) + (len(col_widths) - 1) * 3  # Account for separator widths

    # ANSI escape codes for color
    BLACK = "\033[30m"
    GREEN = "\033[32m"
    RESET = "\033[0m"

    # Create a compact header with everything on one line
    header_line = "=" * (total_width + 4)
    print(BLACK + header_line + RESET)
    print(BLACK + f"{table_name.center(total_width + 4)}" + RESET)
    print(BLACK + header_line + RESET)

    # Print the table header
    header = " | ".join(f"{col:{col_widths[i]}}" for i, col in enumerate(df_combined.columns))
    separator = "-+-".join("-" * width for width in col_widths)

    print(BLACK + header + RESET)
    print(BLACK + separator + RESET)

    # Print each row with aligned columns
    for _, row in df_combined.iterrows():
        row_values = row[1:].apply(pd.to_numeric, errors='coerce')  # Convert to numeric for comparison
        min_value = row_values.min(skipna=True)
        row_data = []
        for i, value in enumerate(row):
            if i == 0:  # First column (Dataset) remains black
                row_data.append(f"{BLACK}{value:{col_widths[i]}}{RESET}")
            elif value == min_value and pd.notna(value):  # Highlight the lowest value in green
                row_data.append(f"{GREEN}{value:{col_widths[i]}}{RESET}")
            else:  # Other values remain black
                row_data.append(f"{BLACK}{value:{col_widths[i]}}{RESET}")
        print(" | ".join(row_data))

    # Add a bottom-line border for the table
    print(BLACK + header_line + RESET)

    # Write the Markdown table to a file
    markdown_table = "| " + " | ".join(column_order) + " |\n"
    markdown_table += "|---" + "|---" * (len(column_order) - 1) + "|\n"
    for _, row in df_combined.iterrows():
        markdown_table += "| " + " | ".join(map(str, row)) + " |\n"

    # Write the Markdown table to the specified output file
    with open(output_file, "w") as f:
        f.write(markdown_table)


def generate_table_4():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "../tables/table_4.md")
    generate_markdown_table(
        input_folder="double",
        output_file=output_file,
        column_order=["Dataset", "Gor", "Ch", "Ch128", "Patas", "PDE", "Elf", "Alp", "LWC+Alp", "Zstd"],
        table_name="Table 4: Compression Ratios for Double Datasets"
    )


def generate_table_7():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "../tables/table_7.md")
    generate_markdown_table(
        input_folder="float",
        output_file=output_file,
        column_order=["Dataset", "Gor", "Ch", "Ch128", "Patas", "Alp", "Zstd"],
        table_name="Table 7: Compression Ratios for Float Datasets"
    )


if __name__ == "__main__":
    generate_table_4()
    generate_table_7()
