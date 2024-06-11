import pandas as pd


def print_f():
    df = pd.read_csv("alp_compression_ratio.csv")
    for index, row in df.iterrows():
        print('{{"{0}", "{1}"}},'.format(row['dataset'], row['size']))


if __name__ == "__main__":
    print_f()
