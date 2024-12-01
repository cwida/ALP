import numpy as np
import pandas as pd

def main():
    # Initial sample array provided
    sample_data = np.array([
        -1.7364531, 1.3814366, 1.438711, 1.155993, 0.010562082, 0.35199697, -0.47133151, -1.028986,
        0.7322407, -1.0070036, 0.23961036, -0.11736983, 1.6188454, 0.87447891, 0.17712031, 0.85670133,
        1.4895166, -0.028453834, 1.1957284, -0.19157237, 1.1785826, -0.51153377, 0.8408226, 1.3257452,
        0.39198864, 1.7005837, 1.3379561, 0.41338141, -0.25926056, 0.46950101, 0.29511222, 1.1901688,
        0.68518363
    ])

    # Replicate the sample to fill an array of size 1024
    replicated_data_1024 = np.resize(sample_data, 1024)
    # Replicate the sample to fill an array of size 100 * 1024 (102400 values)
    replicated_data_102400 = np.resize(sample_data, 100 * 1024)

    # Save the 1024 values CSV without header
    df_1024 = pd.DataFrame(replicated_data_1024)
    df_1024.to_csv("../data/issue/issue_24_1024_values.csv", index=False, header=False)
    print("Data saved to issue_24_1024_values.csv without header")

    # Save the 102400 values CSV without header
    df_102400 = pd.DataFrame(replicated_data_102400)
    df_102400.to_csv("../data/issue/issue_24_102400_values.csv", index=False, header=False)
    print("Data saved to issue_24_102400_values.csv without header")

# Run the main function if the script is executed directly
if __name__ == "__main__":
    main()
