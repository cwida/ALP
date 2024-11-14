import pandas as pd
import glob

def generate_markdown_table():
    # Define the path pattern for the CSV files in the parent directory
    file_pattern = "../../*_compression_ratio.csv"
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
        prefix = file.split("/")[-1].split("_compression_ratio.csv")[0].lower()
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
    column_order = ["Dataset", "Gor", "Ch", "Ch128", "Patas", "PDE", "Elf", "Alp", "LWC+Alp", "Zstd"]
    for col in column_order:
        if col not in df_combined.columns:
            df_combined[col] = ""  # Add empty columns if missing

    # Reorder the columns to match the specified order
    df_combined = df_combined[column_order]

    # Fill NaN with an empty string for better readability in terminal
    df_combined = df_combined.fillna("")

    # Generate the console-friendly table with aligned columns
    col_widths = [max(len(str(value)) for value in df_combined[col].astype(str).tolist() + [col]) + 2 for col in df_combined.columns]
    header = " | ".join(f"{col:{col_widths[i]}}" for i, col in enumerate(df_combined.columns))
    separator = "-+-".join("-" * width for width in col_widths)

    # Print the table header
    print(header)
    print(separator)

    # Print each row with aligned columns
    for _, row in df_combined.iterrows():
        row_data = " | ".join(f"{str(value):{col_widths[i]}}" for i, value in enumerate(row))
        print(row_data)

    # Write the Markdown table to a file
    markdown_table = "| " + " | ".join(column_order) + " |\n"
    markdown_table += "|---" + "|---" * (len(column_order) - 1) + "|\n"
    for _, row in df_combined.iterrows():
        markdown_table += "| " + " | ".join(map(str, row)) + " |\n"

    with open("compression_ratios_table.md", "w") as f:
        f.write(markdown_table)

if __name__ == "__main__":
    generate_markdown_table()
