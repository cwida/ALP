import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import warnings

# Suppress specific warnings
warnings.filterwarnings("ignore", category=pd.errors.SettingWithCopyWarning)
warnings.filterwarnings("ignore", category=UserWarning, message="The PostScript backend does not support transparency")

def clean_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    cleaned_lines = [line.rstrip(',\n') + '\n' for line in lines]
    cleaned_file_path = file_path + '_cleaned'
    with open(cleaned_file_path, 'w') as file:
        file.writelines(cleaned_lines)
    return cleaned_file_path

def plot_figure_6():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, "../end_to_end_bench/result")
    cleaned_file_path = clean_file(file_path)

    column_names = [
        'dataset', 'repetition', 'warmup_repetition', 'scheme', 'thread_n',
        'query', 'time(s)', 'result(tpc)', 'corrected_result(tpc)', 'validity',
        'compression_cycles', 'cycles'
    ]

    df = pd.read_csv(cleaned_file_path, names=column_names, header=0)
    plot_data = df[['dataset', 'scheme', 'thread_n', 'cycles']].copy()
    plot_data['cycles'] = pd.to_numeric(plot_data['cycles'], errors='coerce')

    plot_data = plot_data.groupby(['dataset', 'scheme', 'thread_n'], as_index=False).mean()
    figures_dir = os.path.join(script_dir, "../figures")
    os.makedirs(figures_dir, exist_ok=True)

    unique_datasets = plot_data['dataset'].unique()
    fig, axes = plt.subplots(1, len(unique_datasets), figsize=(20, 4), sharey=True)

    for i, dataset in enumerate(unique_datasets):
        subset = plot_data[plot_data['dataset'] == dataset]
        ax = axes[i]
        sns.barplot(
            data=subset,
            x='scheme',
            y='cycles',
            hue='thread_n',
            ax=ax,
            palette='viridis',
            alpha=1.0  # Disable transparency
        )
        ax.set_title(f"Dataset: {dataset}")
        ax.set_ylabel("Cycles" if i == 0 else "")
        ax.set_xlabel("Schemes")
        ax.tick_params(axis='x', rotation=45)
        if i < len(unique_datasets) - 1:
            ax.legend().remove()

    handles, labels = axes[-1].get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center', ncol=3, title="Threads")
    plt.tight_layout()

    output_path_png = os.path.join(figures_dir, "figure_6.png")
    output_path_eps = os.path.join(figures_dir, "figure_6.eps")
    plt.savefig(output_path_png)
    plt.savefig(output_path_eps, format='eps')
