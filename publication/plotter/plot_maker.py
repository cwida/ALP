import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.font_manager as font_manager
import matplotlib
import numpy as np
import seaborn as sns
import os
import warnings
import matplotlib as mpl
from matplotlib.ticker import AutoMinorLocator

from constants import *

warnings.filterwarnings("ignore", category=pd.errors.SettingWithCopyWarning)
warnings.filterwarnings("ignore", category=UserWarning, message="The PostScript backend does not support transparency")

class PlotMaker:

    matplotlib.font_manager.findSystemFonts(fontpaths=None, fontext='ttf')
    matplotlib.rc('font', family='Droid Serif')

    def __init__(self, results_directory='publication/results', out_directory='publication/figures'):
        self.results_directory = results_directory
        self.out_directory = out_directory
        self.main_arch_directory = 'i4i_4xlarge'

    def get_dataset_name(self, name):
        for datasetName in DATASET_NAMES:
            nameProcessed = '_'.join(name.split('_')[:-1])
            if name == datasetName or nameProcessed == datasetName:
                return DATASET_NAMES[datasetName]
        return name
    
    def map_encoding_name(self, name):
        if name in ENCODINGS_MAPPING:
            return ENCODINGS_MAPPING[name]
        return name

    def clean_end_to_end_file(self, file_path):
        with open(file_path, 'r') as file:
            lines = file.readlines()
        cleaned_lines = [line.rstrip(',\n') + '\n' for line in lines]
        cleaned_lines.append( # TODO: Nasty hack to fill 0 in PDE which cannot compress NYC
            'nyc29_tw,4,0,PDE,1,SCAN,0.000000,0.000000,0.000000,-1.000000,0,0\n'
        )
        cleaned_file_path = file_path + '_cleaned'
        with open(cleaned_file_path, 'w') as file:
            file.writelines(cleaned_lines)
        return cleaned_file_path


    def get_encoding_process(self, name):
        if 'encode' in name:
            return 'Compression'
        else:
            return 'Decompression'


    def get_fused_process(self, name):
        if 'fused' in name:
            return 'Fused'
        else:
            return 'Non-Fused'


    def plot_speed(self):
        architecture = self.main_arch_directory
        basePath = self.results_directory + '/' + architecture + '/'

        patas = pd.read_csv(basePath + 'patas.csv')
        chimp = pd.read_csv(basePath + 'chimp.csv')
        chimp128 = pd.read_csv(basePath + 'chimp128.csv')
        pde = pd.read_csv(basePath + 'ped.csv')
        alp1 = pd.read_csv(basePath + 'x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')
        alp1 = alp1[~(alp1['name'].str.contains('POI-'))]
        alp1 = alp1[~(alp1['name'].str.contains('poi_'))]
        alp2 = pd.read_csv(basePath + 'alp_encode_pde.csv')
        alp3 = pd.read_csv(basePath + 'alp_encode_cutter.csv')
        alp4 = pd.read_csv(basePath + 'alp_decode_cutter.csv')
        gorilla = pd.read_csv(basePath + 'gorillas.csv')
        elf = pd.read_csv(basePath + 'elf_raw.csv')
        zstd = pd.read_csv(basePath + 'zstd.csv')

        alp1 = alp1[(alp1['name'].str.contains('fused')) | alp1['name'].str.contains('decode')]
        alp = pd.concat([alp1, alp2, alp3, alp4])
        alp = alp[~alp['name'].str.contains('bw')]

        # These datasets do not have enough data for Zstd
        zstd = zstd[~zstd['name'].isin([
            'bitcoin_transactions_f_encode',
            'bitcoin_transactions_f_decode',
            'bird_migration_f_encode',
            'bird_migration_f_decode',
            'ssd_hdd_benchmarks_f_encode',
            'ssd_hdd_benchmarks_f_decode'
        ])]

        # This dicitonary determines the order
        benchmarks = {
            'ALP': alp,
            'PDE': pde, 
            'ELF': elf,
            'Zstd': zstd,
            'Patas': patas, 
            'Chimp128': chimp128, 
            'Chimp': chimp, 
            'Gorilla': gorilla,
        }

        for benchmarkName in benchmarks:
            benchmark = benchmarks[benchmarkName]
            if (benchmarkName == 'ELF'):
                elf_encode = benchmark[['dataset', 'compression_tpc']]
                elf_encode.columns = ['dataset', 'tuples_per_cycle']
                elf_encode['algorithm'] = benchmarkName
                elf_encode['process'] = 'Compression'
                elf_decode = benchmark[['dataset', 'decompression_tpc']]
                elf_decode.columns = ['dataset', 'tuples_per_cycle']
                elf_decode['algorithm'] = benchmarkName
                elf_decode['process'] = 'Decompression'
                benchmarks[benchmarkName] = pd.concat([elf_encode, elf_decode])
                continue
            benchmark['tuples_per_cycle'] = 1 / benchmark['cycles_per_tuple']
            benchmark['dataset'] = benchmark['name'].apply(self.get_dataset_name)
            benchmark['process'] = benchmark['name'].apply(self.get_encoding_process)
            benchmark['algorithm'] = benchmarkName
            benchmarks[benchmarkName] = benchmark[['tuples_per_cycle', 'dataset', 'process', 'algorithm']]
        
        mergedBenchmarks = pd.concat(benchmarks.values())
        # Air sensor is a synthetic dataset
        mergedBenchmarks =  mergedBenchmarks[mergedBenchmarks['dataset'] != 'Air-sensor']

        compression_all = mergedBenchmarks[
            mergedBenchmarks['process'] == 'Compression'
        ]
        decompression_all = mergedBenchmarks[
            mergedBenchmarks['process'] == 'Decompression'
        ]
        compression_all['tuples_per_cycle_decomp'] = decompression_all['tuples_per_cycle'].values


        font = {'size': 8}
        matplotlib.rc('font', **font)

        fig, (ax1) = plt.subplots(1, 1)
        fig.set_size_inches(4.5, 2.2)

        sns.scatterplot(
            data=compression_all, 
            x="tuples_per_cycle", 
            y="tuples_per_cycle_decomp", 
            ax=ax1,
            s=150,
            marker=".",
            linewidth=0.4,
            hue='algorithm',
            palette=ALGORITHM_COLORS,
            alpha=1
        )  

        ax1.xaxis.grid(
            linewidth=0.5,
            color='#ededed',
        )

        ax1.yaxis.grid(
            linewidth=0.5,
            color='#ededed',
        )

        ax1.set_xlabel('Compression Speed\nas Tuples per CPU Cycle (Log Scale)', fontdict={"size": 8})
        ax1.set_ylabel('Decompression Speed\nas Tuples per CPU Cycle (Log Scale)', fontdict={"size": 8})

        ax1.set_yscale('log')
        ax1.set_xscale('log')

        ax1.set_axisbelow(True)

        ax1.text(
            0.05, 0.01,
            "1.7x", 
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['Chimp']
        )

        ax1.text(
            0.12, 0.03,
            "2.2x", 
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['Chimp128']
        )

        ax1.text(
            0.07, 0.40,
            "1.8x", 
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['Patas']
        )

        ax1.text(
            0.003, 0.14,
            "2.0x",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['PDE']
        )

        ax1.text(
            0.04, 0.06,
            "1.5x",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['Gorilla']
        )

        ax1.text(
            0.004, 0.01,
            "2.8x",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['ELF']
        )

        ax1.text(
            0.011, 0.09,
            "3.1x",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['Zstd']
        )

        ax1.text(
            0.35, 2.5,
            "Compression Ratio:\n3.0x", #"Compression Ratio: 23.7\nBits per Value",
            fontsize=8.5,
            horizontalalignment='right',
            fontweight='bold',
            color=ALGORITHM_COLORS['ALP']
        )


        handles, labels = ax1.get_legend_handles_labels()
        handles, labels
        order = [4,3,0,2,1,5]

        ax1.legend(
            loc="upper left",
            prop={'size': 6.5},
            frameon=False,
            ncols=2
        )

        ax1.set_ylim((0, 13))

        plt.savefig(f'{self.out_directory}/figure_1_Speed.eps', format='eps', dpi=800, bbox_inches='tight')
        plt.savefig(f'{self.out_directory}/figure_1_Speed.png', format='png', dpi=800, bbox_inches='tight')


    def plot_fused_unfused(self):
        df_bw = pd.read_csv(f'{self.results_directory}/{self.main_arch_directory}/x86_64_avx512bw_intrinsic_1024_uf1_falp_bw.csv')
        df_bw = df_bw[df_bw['name'].str.contains('bw')]
        df_bw['process'] = df_bw['name'].apply(self.get_fused_process)
        df_bw['tuples_per_cycle'] = 1 / df_bw['cycles_per_tuple']
        df_bw = df_bw[['tuples_per_cycle', 'process', 'benchmark_number']]
        
        df = pd.read_csv(f'{self.results_directory}/{self.main_arch_directory}/x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')
        df = df[~df['name'].str.contains('bw')]
        df['tuples_per_cycle'] = 1 / df['cycles_per_tuple']
        df['process'] = df['name'].apply(self.get_fused_process)
        df = df[
            ~df['name'].str.contains('POI-')
        ]
        df = df[
            ~df['name'].str.contains('poi_')
        ]

        df = df[['tuples_per_cycle', 'process']]
        dummylist = []
        for i in range(len(df) // 2):
            for j in range(2):
                dummylist.append(i)
            
        df['dummy'] = dummylist

        font = {'size': 8}
        matplotlib.rc('font', **font)

        fig, (ax1u, ax1d, ax2u, ax2d) = plt.subplots(4, 1, constrained_layout=False, gridspec_kw={'height_ratios': [1, 3, 1, 3]})
        fig.set_size_inches(4.5, 3.4)

        plt.subplots_adjust(hspace=0.6)

        sns.scatterplot(
            data=df, 
            x="dummy", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax1u,
            s=20,
            #style='process',
            linewidth=0.3,
            edgecolor='white',
            palette=F_NF_COLORS
        )

        sns.scatterplot(
            data=df, 
            x="dummy", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax1d,
            s=20,
            #style='process',
            linewidth=0.3,
            edgecolor='white',
            palette=F_NF_COLORS
        )

        ax1u.spines['bottom'].set_visible(False)
        ax1d.spines['top'].set_visible(False)

        ax1u.xaxis.tick_top()
        ax1u.tick_params(labeltop=False)
        ax1d.xaxis.tick_bottom()

        kwargs = dict(transform=ax1u.transAxes, color='k', clip_on=False)
        d = .007
        ax1u.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
        ax1u.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

        kwargs.update(transform=ax1d.transAxes)  # switch to the bottom axes
        ax1d.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
        ax1d.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal

        ax1u.set_xlabel('')
        ax1u.set_ylabel('')
        ax2u.set_ylabel('')

        ax1u.set_ylim(8.5, 9.5)
        ax1d.set_ylim(0, 2.5)

        ax1d.xaxis.labelpad = 0.5

        sns.lineplot(
            data=df_bw[0:-24], 
            x="benchmark_number", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax2u,
            linewidth=0.6,
            markers=False,
            palette=F_NF_COLORS,
            dashes=False,
            legend=False
        )

        sns.scatterplot(
            data=df_bw[0:-24], 
            x="benchmark_number", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax2u,
            s=70,
            marker=".",
            linewidth=0.2,
            edgecolor='white',
            palette=F_NF_COLORS
        )

        sns.lineplot(
            data=df_bw[0:-24], 
            x="benchmark_number", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax2d,
            linewidth=0.6,
            markers=False,
            palette=F_NF_COLORS,
            dashes=False,
            legend=False
        )

        sns.scatterplot(
            data=df_bw[0:-24], 
            x="benchmark_number", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax2d,
            s=30,
            marker=".",
            linewidth=0.2,
            edgecolor='white',
            palette=F_NF_COLORS
        )

        ax2u.spines['bottom'].set_visible(False)
        ax2d.spines['top'].set_visible(False)

        ax2u.xaxis.tick_top()
        ax2u.tick_params(labeltop=False)
        ax2d.xaxis.tick_bottom()

        kwargs = dict(transform=ax2u.transAxes, color='k', clip_on=False)
        d = .007
        ax2u.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
        ax2u.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

        kwargs.update(transform=ax2d.transAxes)  # switch to the bottom axes
        ax2d.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
        ax2d.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal

        ax2u.set_xlabel('')
        ax2u.set_ylabel('')
        ax2u.set_ylabel('')

        ax1d.set_xticks([])

        ax1u.set_ylim(8.5, 10)
        ax1d.set_ylim(0, 2.5)

        ax2u.set_ylim(8.5, 10)
        ax2d.set_ylim(0, 2.5)

        ax1u.xaxis.grid(linewidth=0.5, color='#ededed')
        ax1u.yaxis.grid(linewidth=0.5, color='#ededed')
        ax1u.set_axisbelow(True)
        ax1d.xaxis.grid(linewidth=0.5, color='#ededed')
        ax1d.yaxis.grid(linewidth=0.5, color='#ededed')
        ax1d.set_axisbelow(True)

        # ax1.set_ylim(0, 10)

        ax1d.set_xticks(range(len(df)//2))
        ax1d.set_xticklabels(
            ['' for i in range(len(df)//2)], 
            rotation=45 
        )

        ax1u.set_xticks(range(len(df)//2))
        ax1u.set_xticklabels(
            ['' for i in range(len(df)//2)], 
            rotation=45 
        )

        ax2u.xaxis.grid(linewidth=0.5, color='#ededed')
        ax2u.yaxis.grid(linewidth=0.5, color='#ededed')
        ax2u.set_axisbelow(True)
        ax2d.xaxis.grid(linewidth=0.5, color='#ededed')
        ax2d.yaxis.grid(linewidth=0.5, color='#ededed')
        ax2d.set_axisbelow(True)

        ax1u.legend(
            loc="upper right",
            prop={'size': 6.5},
            frameon=False,
            bbox_to_anchor=(1,1)
        )

        ax1d.legend().remove()

        ax2u.legend(
            loc="upper right",
            prop={'size': 6.5},
            frameon=False,
            bbox_to_anchor=(1,1)
        )
        ax2d.legend().remove()

        ax2u.set_axisbelow(True)
        ax2d.set_axisbelow(True)

        fig.supylabel('Decompression Speed\nTuples per CPU Cycle', verticalalignment='center', horizontalalignment='center', y=0.48, fontsize=8)
        ax1d.set_xlabel('Datasets', fontsize=8)
        ax1d.set_ylabel('')


        x2labels = []
        for i in range (0, 54):
            if i % 4 == 0:
                x2labels.append(i)

        ax2d.set_xticks(x2labels)
        ax2d.set_xticklabels(
            x2labels, 
            rotation=0 
        )
        ax2u.set_xticks(x2labels)
        ax2u.set_xticklabels(
            x2labels, 
            rotation=0 
        )
        ax2u.set_ylabel('')
        ax2d.set_ylabel('')

        ax2u.xaxis.grid(linewidth=0.5, color='#ededed')
        ax2d.yaxis.grid(linewidth=0.5, color='#ededed')
        ax2u.xaxis.grid(linewidth=0.5, color='#ededed')
        ax2d.yaxis.grid(linewidth=0.5, color='#ededed')


        ax1u.tick_params(axis='x', colors='white')
        ax2u.tick_params(axis='x', colors='white')

        ax2d.set_xlabel('Bitwidth of Vector', fontsize=8)
        plt.savefig(f'{self.out_directory}/figure_5_fusing.eps', format='eps', dpi=800, bbox_inches='tight')
        plt.savefig(f'{self.out_directory}/figure_5_fusing.png', format='png', dpi=800, bbox_inches='tight')


    def plot_architectures(self):
        graviton2 = [
            pd.read_csv(f'{self.results_directory}/c6g/fallback_scalar_aav_1024_uf1_falp.csv'),   # Auto-Vectorized
            pd.read_csv(f'{self.results_directory}/c6g/fallback_scalar_nav_1024_uf1_falp.csv'),   # Scalar
            pd.read_csv(f'{self.results_directory}/c6g/arm64v8_neon_intrinsic_1024_uf1_falp.csv')# SIMDized
        ]
        graviton3 = [
            pd.read_csv(f'{self.results_directory}/c7g/fallback_scalar_aav_1024_uf1_falp.csv'),   # Auto-Vectorized
            pd.read_csv(f'{self.results_directory}/c7g/fallback_scalar_nav_1024_uf1_falp.csv'),   # Scalar
            pd.read_csv(f'{self.results_directory}/c7g/arm64v8_neon_intrinsic_1024_uf1_falp.csv')# SIMDized
        ]
        icelake = [
            pd.read_csv(f'{self.results_directory}/i4i_4xlarge/fallback_scalar_aav_1024_uf1_falp.csv'),   # Auto-Vectorized
            pd.read_csv(f'{self.results_directory}/i4i_4xlarge/fallback_scalar_nav_1024_uf1_falp.csv'),   # Scalar
            pd.read_csv(f'{self.results_directory}/i4i_4xlarge/x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')# SIMDized
        ]
        m1 = [
            pd.read_csv(f'{self.results_directory}/m1/fallback_scalar_aav_1024_uf1_falp.csv'),   # Auto-Vectorized
            pd.read_csv(f'{self.results_directory}/m1/fallback_scalar_nav_1024_uf1_falp.csv'),   # Scalar
            pd.read_csv(f'{self.results_directory}/m1/arm64v8_neon_intrinsic_1024_uf1_falp.csv')# SIMDized
        ]
        zen3 = [
            pd.read_csv(f'{self.results_directory}/m6a_xlarge/fallback_scalar_aav_1024_uf1_falp.csv'),   # Auto-Vectorized
            pd.read_csv(f'{self.results_directory}/m6a_xlarge/fallback_scalar_nav_1024_uf1_falp.csv'),   # Scalar
            pd.read_csv(f'{self.results_directory}/m6a_xlarge/x86_64_avx2_intrinsic_1024_uf1_falp.csv') # SIMDized
        ]

        architecturesRaw = [graviton2, graviton3, icelake, m1, zen3]

        method = ['Auto-Vectorized', 'Scalar', 'SIMDized']
        architectures = ['Graviton2', 'Graviton3', 'Ice Lake', 'M1', 'Zen3']
        dfArch = []
        for i, arch in enumerate(architecturesRaw):
            for j, df in enumerate(arch):
                df['arch'] = architectures[i]
                df['method'] = method[j]
                dfArch.append(df)
        df = pd.concat(dfArch)
        df['tuples_per_cycle'] = 1 / df['cycles_per_tuple']
        df = df[
            (df['name'].str.contains('fused'))
            & (~df['name'].str.contains('bw'))
            & (~df['name'].str.contains('gov'))
        ]
        df = df[['tuples_per_cycle', 'arch', 'method']]

        font = {'size': 8}
        matplotlib.rc('font', **font)

        fig, (ax1) = plt.subplots(1, 1, constrained_layout=True)
        fig.set_size_inches(4.5, 1.8)

        sns.stripplot(
            data=df, 
            x="arch", 
            y="tuples_per_cycle", 
            hue='method',
            jitter=0.3,
            ax=ax1,
            s=9,
            marker=".",
            linewidth=0.3,
            edgecolor='white',
            palette=CODE_COLORS
        )

        ax1.xaxis.grid(linewidth=0.5, color='#ededed')
        ax1.yaxis.grid(linewidth=0.5, color='#ededed')

        ax1.set_xlabel('')
        ax1.set_ylabel('')

        ax1.legend(
            loc="upper left",
            prop={'size': 6.5},
            frameon=False,
            bbox_to_anchor=(0.72,1)
        )

        ax1.set_ylabel('Decompression Speed\nTuples per CPU Cycle')
        ax1.set_xlabel('Architectures')

        plt.savefig(f'{self.out_directory}/figure_4_Architectures.eps', format='eps', dpi=800, bbox_inches='tight')
        plt.savefig(f'{self.out_directory}/figure_4_Architectures.png', format='png', dpi=800, bbox_inches='tight')

    def plot_end_to_end(self):
        mpl.rcParams['hatch.linewidth'] = 0.2
        mpl.rcParams['axes.linewidth'] = 0.5 

        script_dir = os.path.dirname(os.path.abspath(__file__))
        file_path = os.path.join(script_dir, "../end_to_end_bench/result")
        cleaned_file_path = self.clean_end_to_end_file(file_path)

        column_names = [
            'dataset', 'repetition', 'warmup_repetition', 'scheme', 'thread_n',
            'query', 'time(s)', 'result(tpc)', 'corrected_result(tpc)', 'validity',
            'compression_cycles', 'cycles'
        ]

        benchmarked_data = pd.read_csv(cleaned_file_path, names=column_names, header=0)
        scan_sum_data = benchmarked_data[['dataset', 'scheme', 'thread_n', 'cycles', 'repetition', 'query']].copy()
        scan_sum_data['cycles'] = pd.to_numeric(scan_sum_data['cycles'], errors='coerce')

        scan_sum_data_w_cycles = scan_sum_data.copy()
        scan_sum_data_w_cycles['cpt'] = ((scan_sum_data_w_cycles['cycles'] / scan_sum_data_w_cycles['repetition'])) * scan_sum_data_w_cycles['thread_n']
        scan_sum_data_w_cycles['tpc'] = 1 / scan_sum_data_w_cycles['cpt']

        comp_data = pd.read_csv(cleaned_file_path, names=column_names, header=0)
        comp_data = comp_data[
            (comp_data['thread_n'] == 1) & 
            (comp_data['query'] == 'SCAN')
        ]
        comp_data['cycles'] = pd.to_numeric(comp_data['compression_cycles'], errors='coerce')
        comp_data['cpt'] = ((comp_data['cycles'] / comp_data['repetition'])) * comp_data['thread_n']
        comp_data['tpc'] = 1 / comp_data['cpt']
        comp_data['query'] = 'COMP'

        benchmarked_data = pd.concat([scan_sum_data_w_cycles, comp_data])
        benchmarked_data['dataset'] = benchmarked_data['dataset'].apply(self.get_dataset_name)
        benchmarked_data['scheme'] = benchmarked_data['scheme'].apply(self.map_encoding_name)
        benchmarked_data['query'] = benchmarked_data['query'] + benchmarked_data['thread_n'].astype(str)
        benchmarked_data = benchmarked_data[['dataset', 'scheme', 'query', 'tpc', 'cpt']]
        benchmarked_data = benchmarked_data.sort_values('scheme')

        # Table 6 Generation (City-Temp end-to-end speed)
        METRIC_FOR_TABLE = 'cpt'
        city_temp_data = benchmarked_data.copy()
        city_temp_data = city_temp_data.sort_values(['scheme', 'dataset', 'query'])
        city_temp_data = city_temp_data.set_index(['scheme', 'dataset'])[['query', METRIC_FOR_TABLE]].pivot(columns=['query'])
        city_temp_data = city_temp_data.droplevel(0, axis=1)
        city_temp_data = city_temp_data.reset_index()
        city_temp_data = city_temp_data.fillna(0)
        city_temp_data = city_temp_data[city_temp_data['dataset'] == 'City-Temp']
        city_temp_data = city_temp_data.set_index('scheme')
        city_temp_data = city_temp_data[['SCAN1', 'SCAN8', 'SCAN16', 'SUM1', 'SUM8',  'SUM16', 'COMP1']]
        print('Table 6 with Raw cycle values', city_temp_data)
        city_temp_data.loc[:,:] = city_temp_data.loc[:,:].div(city_temp_data.iloc[0, :])
        city_temp_data.loc[['ALP', 'Uncompressed', 'PDE', 'Patas', 'Gorilla', 'Chimp', 'Chimp128', 'Zstd']]
        print('Table 6 normalized', city_temp_data)
        city_temp_data = city_temp_data.astype(str)
        city_temp_data = city_temp_data.map(lambda x: x[:4]) # 4 precision digits
        city_temp_data.iloc[1] = city_temp_data.iloc[1].astype(str) + 'x Slower than ALP ↓'
        output_file = os.path.join(script_dir, "../tables/table_6.md")
        city_temp_data.to_markdown(output_file)
        ################
        
        METRIC_TO_PLOT = 'cpt'
        benchmarked_data = benchmarked_data.sort_values(['scheme', 'dataset', 'query'])
        benchmarked_data = benchmarked_data.set_index(['scheme', 'dataset'])[['query', METRIC_TO_PLOT]].pivot(columns=['query'])
        benchmarked_data = benchmarked_data.droplevel(0, axis=1)
        benchmarked_data = benchmarked_data.reset_index()
        benchmarked_data = benchmarked_data.fillna(0)
        data_for_plots = benchmarked_data.copy()
        data_for_plots['SUM-SCAN1'] = data_for_plots['SUM1'] - data_for_plots['SCAN1']
        data_for_plots['SUM-SCAN8'] = (data_for_plots['SUM8'] - data_for_plots['SCAN8'])
        data_for_plots['SUM-SCAN16'] = (data_for_plots['SUM16'] - data_for_plots['SCAN16'])

        datasets_of_interest = [
            "Gov/26",
            "City-Temp",
            "Food-prices",
            "Blockchain-tr",
            "NYC/29"
        ]

        datasets_bits_value = [
            "0.4",
            "10.7",
            "23.7",
            "36.2",
            "40.4"
        ]

        data_for_plots = data_for_plots[data_for_plots['dataset'].isin(
            datasets_of_interest
        )]

        fig, ((ax1, ax2, ax3, ax4, ax5)) = plt.subplots(1, 5, constrained_layout=False)
        fig.set_size_inches(11, 1.2)

        all_axes = [ax1, ax2, ax3, ax4, ax5]

        data_for_plots = data_for_plots.set_index('dataset')

        queries = ['SUM1', 'SCAN1', 'SUM8', 'SCAN8', 'SUM16', 'SCAN16', 'COMP1']
        alg_order = ['ALP', 'Uncompressed', 'PDE', 'Patas', 'Gorilla', 'Chimp', 'Chimp128', 'Zstd']
        labels_for_plot = ['ALP', 'Unc.', 'PDE', 'Patas', 'Gor.', 'Ch.', 'Ch.128', 'Zstd']

        for i in range(len(all_axes)):
            query = queries[i]
            ax = all_axes[i]
            cur_dataset_name = datasets_of_interest[i]
            
            cur_dataset = data_for_plots.loc[cur_dataset_name, :]
            cur_dataset = cur_dataset.set_index('scheme')
            cur_dataset = cur_dataset.loc[alg_order]
            
            cur_dataset_real_sum = cur_dataset[['SUM-SCAN1', 'SUM-SCAN8', 'SUM-SCAN16']]
            cur_dataset_sum = cur_dataset[['SUM1', 'SUM8', 'SUM16']]
            
            cur_dataset_sum.plot.bar(
                ax=ax,
                color=END_TO_END_COLORS,
                linewidth=0.2,
                edgecolor='black',
            )
            
            cur_dataset_real_sum.plot.bar(
                ax=ax,
                color=END_TO_END_COLORS,
                linewidth=0.2,
                edgecolor='black',
                hatch='///////'
            )
            
            minor_locator = AutoMinorLocator(2)
            ax.xaxis.set_minor_locator(minor_locator)
            
            ax.xaxis.grid(
                which='minor',
                color='#e6e6e6',
                linewidth=0.5,
                alpha=1
            )
            ax.tick_params(axis='x', which='minor', colors='white')
            
            ax.yaxis.grid(
                color='#e6e6e6',
                linewidth=0.5,
                alpha=1
            )

            ax.set_xlabel('')
            ax.set_ylabel('')
            ax.tick_params(axis='x', labelrotation=0, labelsize=5)
            ax.set_xticklabels(labels_for_plot)
            ax.tick_params(axis='y', labelsize=6)
            
            if i == 0:
                subplot_title = cur_dataset_name + " (" + datasets_bits_value[i] + " bits/value on ALP)"
            else:
                subplot_title = cur_dataset_name + " (" + datasets_bits_value[i] + " bits/value)"
            
            ax.set_title(subplot_title, size=7)
            ax.set_axisbelow(True)
            
            if i != 0:
                ax.get_legend().remove()
            
            # PDE cannot compress NYC/29
            if (cur_dataset_name == 'NYC/29'):
                ax.plot(2, 1.5, marker='x', color='red')
                
            ax.set_ylim(ymin=0)
            
        fig.supylabel('CPU Cycles\n[$\it{Lower=Better}$]\n(Log Scale)', size=8, ha='center')
        fig.supylabel('CPU Cycles\n[$\it{Lower=Better}$]', size=8, ha='center')

        plt.subplots_adjust(left=0.06, hspace=0.4)

        handles, labels = ax1.get_legend_handles_labels()
        new_labels = ['SCAN 1 thread', 'SCAN 8 threads', 'SCAN 16 threads', 'SUM']
        ax1.legend(
            handles[0:5],
            new_labels,
            loc="upper left",
            prop={'size': 5},
            frameon=False,
            ncols=1,
            handlelength=0.7
        )

        figures_dir = os.path.join(script_dir, "../figures")
        output_path_png = os.path.join(figures_dir, "figure_6_EndToEnd.png")
        output_path_eps = os.path.join(figures_dir, "figure_6_EndToEnd.eps")

        plt.savefig(output_path_png, format='png', dpi=600, bbox_inches='tight')
        plt.savefig(output_path_eps, format='eps', dpi=800, bbox_inches='tight')