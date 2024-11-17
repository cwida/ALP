import pandas as pd
import glob
import os


def generate_sorted_markdown_table(input_folder, output_file, column_order, row_order, table_name, calculate_extra_rows=False):
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
        "alp_rd": "Alp_rd",
        "lwc_alp": "LWC+Alp",
        "zstd": "Zstd"
    }

    # Initialize an empty DataFrame to hold all datasets
    df_combined = pd.DataFrame()

    # Load data from each file and insert it into the correct column
    for file in csv_files:
        prefix = file.split("/")[-1].split(".csv")[0].lower()
        if prefix in file_to_column:
            column_name = file_to_column[prefix]
            df = pd.read_csv(file, usecols=["dataset", "size"])  # Only load the necessary columns
            df = df.rename(columns={"size": column_name})
            df.set_index("dataset", inplace=True)
            df_combined = df_combined.combine_first(df) if not df_combined.empty else df

    # Reset the index and rename it to "Dataset"
    df_combined.reset_index(inplace=True)
    df_combined.rename(columns={"dataset": "Dataset"}, inplace=True)

    # Filter out rows with missing dataset names
    df_combined = df_combined[df_combined["Dataset"].notna()]

    # Combine Alp and Alp_rd into a single column, taking the non-null value
    if "Alp" in df_combined.columns and "Alp_rd" in df_combined.columns:
        df_combined["Alp"] = df_combined["Alp"].combine_first(df_combined["Alp_rd"])
        df_combined.drop(columns=["Alp_rd"], inplace=True)

    # Ensure all expected columns are included in the specified order
    for col in column_order:
        if col not in df_combined.columns:
            df_combined[col] = 64  # Add columns with default value 64 if missing

    # Add missing rows with default values
    for dataset in row_order:
        if dataset not in df_combined["Dataset"].values:
            empty_row = {col: 64 for col in column_order}
            empty_row["Dataset"] = dataset
            df_combined = pd.concat([df_combined, pd.DataFrame([empty_row])], ignore_index=True)

    # Reorder rows to match the order of `row_order`
    df_combined["Dataset"] = pd.Categorical(df_combined["Dataset"], categories=row_order, ordered=True)
    df_combined = df_combined.sort_values("Dataset").reset_index(drop=True)

    # Replace NaN values with 64 in numeric columns
    numeric_columns = df_combined.columns.difference(["Dataset"])
    df_combined[numeric_columns] = df_combined[numeric_columns].apply(pd.to_numeric, errors='coerce').fillna(64)

    # Add TS AVG., NON-TS, and ALL AVG rows only if required
    if calculate_extra_rows:
        if "Wind-dir" in df_combined["Dataset"].values:
            wind_dir_index = df_combined[df_combined["Dataset"] == "Wind-dir"].index[0]
            ts_avg = df_combined.iloc[:wind_dir_index + 1, 1:].mean().round(1)
            ts_avg_row = pd.DataFrame([["TS AVG."] + ts_avg.tolist()], columns=df_combined.columns)
            df_combined = pd.concat(
                [df_combined.iloc[:wind_dir_index + 1], ts_avg_row, df_combined.iloc[wind_dir_index + 1:]],
                ignore_index=True
            )

        if "Arade/4" in df_combined["Dataset"].values and "SD-bench" in df_combined["Dataset"].values:
            arade_index = df_combined[df_combined["Dataset"] == "Arade/4"].index[0]
            sd_bench_index = df_combined[df_combined["Dataset"] == "SD-bench"].index[0]
            non_ts_avg = df_combined.iloc[arade_index:sd_bench_index + 1, 1:].mean().round(1)
            non_ts_row = pd.DataFrame([["NON-TS"] + non_ts_avg.tolist()], columns=df_combined.columns)
            df_combined = pd.concat([df_combined, non_ts_row], ignore_index=True)

        # Calculate ALL AVG excluding TS AVG. and NON-TS
        avg_rows = df_combined[~df_combined["Dataset"].isin(["TS AVG.", "NON-TS"])]
        all_avg = avg_rows.iloc[:, 1:].mean().round(1)
        all_avg_row = pd.DataFrame([["ALL AVG."] + all_avg.tolist()], columns=df_combined.columns)
        df_combined = pd.concat([df_combined, all_avg_row], ignore_index=True)

    # Keep only the columns specified in column_order
    df_combined = df_combined[column_order]

    # Generate the console-friendly table with aligned columns
    col_widths = [max(len(str(value)) for value in df_combined[col].astype(str).tolist() + [col]) + 2 for col in
                  df_combined.columns]
    total_width = sum(col_widths) + (len(col_widths) - 1) * 3  # Account for separator widths

    # ANSI escape codes for color and formatting
    BLACK = "\033[30m"
    GREEN = "\033[32m"
    BOLD = "\033[1m"
    RESET = "\033[0m"

    # Print header
    print("=" * (total_width + 4))
    print(f"{table_name.center(total_width + 4)}")
    print("=" * (total_width + 4))

    # Print the table header
    header = " | ".join(f"{col:{col_widths[i]}}" for i, col in enumerate(df_combined.columns))
    separator = "-+-".join("-" * width for width in col_widths)

    print(header)
    print(separator)

    # Print each row with aligned columns
    for _, row in df_combined.iterrows():
        row_values = row[1:].apply(pd.to_numeric, errors='coerce')  # Convert to numeric for comparison
        min_value = row_values.min(skipna=True)
        row_data = []
        bold_row = row["Dataset"] in ["TS AVG.", "NON-TS", "ALL AVG."]  # Check if the row is bold
        for i, value in enumerate(row):
            if i == 0:  # First column (Dataset) remains black
                row_text = f"{value:{col_widths[i]}}"
                row_data.append(f"{BOLD}{row_text}{RESET}" if bold_row else f"{BLACK}{row_text}{RESET}")
            elif value == min_value and pd.notna(value):  # Highlight the lowest value in green
                row_text = f"{value:{col_widths[i]}}"
                row_data.append(f"{GREEN}{row_text}{RESET}")
            else:  # Other values remain black
                row_text = f"{value:{col_widths[i]}}"
                row_data.append(f"{BOLD}{row_text}{RESET}" if bold_row else f"{BLACK}{row_text}{RESET}")
        print(" | ".join(row_data))

    # Add a bottom-line border for the table
    print("=" * (total_width + 4))

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
    generate_sorted_markdown_table(
        input_folder="double",
        output_file=output_file,
        column_order=["Dataset", "Gor", "Ch", "Ch128", "Patas", "PDE", "Elf", "Alp", "LWC+Alp", "Zstd"],
        row_order=[
            "Air-Pressure", "Basel-Temp", "Basel-Wind", "Bird-Mig", "Btc-Price",
            "City-Temp", "Dew-Temp", "Bio-Temp", "PM10-dust", "Stocks-DE",
            "Stocks-UK", "Stocks-USA", "Wind-dir", "Arade/4", "Blockchain",
            "CMS/1", "CMS/25", "CMS/9", "Food-prices", "Gov/10", "Gov/26",
            "Gov/30", "Gov/31", "Gov/40", "Medicare/1", "Medicare/9", "NYC/29",
            "POI-lat", "POI-lon", "SD-bench"
        ],
        table_name="Table 4: Compression Ratios for Double Datasets",
        calculate_extra_rows=True
    )


def generate_table_7():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "../tables/table_7.md")
    generate_sorted_markdown_table(
        input_folder="float",
        output_file=output_file,
        column_order=["Dataset", "Gor", "Ch", "Ch128", "Patas", "Alp", "Zstd"],
        row_order=["Dino-Vitb16", "GPT2", "Grammarly-lg", "W2V Tweets"],
        table_name="Table 7: Compression Ratios for Float Datasets",
        calculate_extra_rows=False  # Skip extra rows for Table 7
    )


if __name__ == "__main__":
    generate_table_4()
    generate_table_7()
