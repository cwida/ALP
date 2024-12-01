# Using Your Own Data in ALP Benchmarks

This guide explains how to set up and benchmark your own dataset using ALP.

---

## Step 1: Understand the Dataset Configuration Format

The dataset configuration is provided in a [CSV file](benchmarks/your_own_dataset.csv), where each row describes a column in your dataset.

Below is the explanation of each parameter:

### Example:
```csv
id,column_name,data_type,path,file_type
0,CLOUDf48.bin.f32,float,/Users/azim/CLionProjects/ALP/100x500x500/CLOUDf48.bin.f32,binary
```

### Parameters:
1. **`id`**:
   - A unique integer identifier for the column.
   - Example: `0`

2. **`column_name`**:
   - A descriptive name for the column.
   - Example: `CLOUDf48.bin.f32`

3. **`data_type`**:
   - The type of data in the column.
   - Allowed values: `float`, `double`
   - Example: `float`

4. **`path`**:
   - The absolute path to the data file for the column.
   - Example: `/Users/azim/CLionProjects/ALP/100x500x500/CLOUDf48.bin.f32`

5. **`file_type`**:
   - The format of the data file.
   - Allowed values: `binary`, `csv`
   - Example: `binary`

---

## Step 2: Create Your Dataset Configuration File

Edit the [CSV file](benchmarks/your_own_dataset.csv) to define your dataset using the format described above.

### Example:
```csv
id,column_name,data_type,path,file_type
0,AnotherDoubleColumn,double,/Users/azim/CLionProjects/ALP/another_double_column.csv,csv
1,AnotherFloatColumn,float,/Users/azim/CLionProjects/ALP/another_float_column.csv,binary
```

---

## Step 3: Build ALP with Benchmarking Enabled

To enable benchmarking in ALP:

1. Configure the build using CMake with the `ALP_BUILD_BENCHMARKING` option set to `ON`:
   ```bash
   cmake -DALP_BUILD_BENCHMARKING=ON -S . -B build
   ```

2. Build the project:
   ```bash
   cmake --build build
   ```

---

## Step 4: Run the Benchmark

Run the benchmark executable:

```bash
cd build
./benchmarks/bench_your_dataset
```

---

## Step 5: Analyze the Results

The benchmark tool will save the results [here](benchmarks/your_own_dataset_result.csv).

The results include the following columns:
- **`idx`**: Column index.
- **`column`**: Column name.
- **`data_type`**: Data type (`float`, `double`).
- **`size`**: Number of bits used to encode this dataset per value.
- **`rowgroups_count`**: Number of row groups. A row group is composed of 100 vectors.
- **`vectors_count`**: Number of vectors. A vector always has 1024 values.

---

### Notes

1. Ensure all file paths in your dataset configuration are valid and accessible.
2. Verify that the `data_type` and `file_type` values in the CSV match the format of your data files.
3. If benchmarking fails, check the logs for errors such as missing files or unsupported formats.

---

By following these steps, you can configure and benchmark your own datasets in ALP, allowing you to evaluate ALP's performance with your data.

We would love to hear about your data and results, so please share them with us. Your feedback can help improve ALP further.

