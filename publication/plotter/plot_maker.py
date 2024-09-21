import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.font_manager as font_manager
import matplotlib
import numpy as np
import seaborn as sns

from constants import *


class PlotMaker:

    matplotlib.font_manager.findSystemFonts(fontpaths=None, fontext='ttf')
    matplotlib.rc('font', family='Droid Serif')

    def __init__(self, results_directory='../results', out_directory='./figures'):
        self.results_directory = results_directory
        self.out_directory = out_directory

    def get_dataset_name(self, name):
        for datasetName in DATASET_NAMES:
            nameProcessed = '_'.join(name.split('_')[:-1])
            if name == datasetName or nameProcessed == datasetName:
                return DATASET_NAMES[datasetName]
        return 'hmm'


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
        architecture = 'i4i'
        basePath = self.results_directory + '/' + architecture + '/'

        patas = pd.read_csv(basePath + 'patas.csv')
        chimp = pd.read_csv(basePath + 'chimp.csv')
        chimp128 = pd.read_csv(basePath + 'chimp128.csv')
        pde = pd.read_csv(basePath + 'ped.csv')
        alp1 = pd.read_csv(basePath + 'x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')
        alp1 = alp1[~(alp1['name'].str.contains('poi_'))]
        alp2 = pd.read_csv(basePath + 'alp_encode.csv')
        alp3 = pd.read_csv(basePath + 'alp_encode_cutter.csv')
        alp4 = pd.read_csv(basePath + 'alp_decode_cutter.csv')
        gorilla = pd.read_csv(basePath + 'gorillas.csv')
        elf = pd.read_csv(basePath + 'elf.csv')
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
            if (benchmarkName not in ['ELF']):
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


        colors = {
            'Chimp': '#f6b26b',
            'Chimp128': 'red',
            'Patas': 'orchid',
            'PDE': '#eb9fa0', 
            'ALP': '#59b872',
            'Gorilla': '#4b8bf5',
            'ELF': '#a9b7c6',
            'Zstd': '#2B3D41',
        }

        markers = {
            'Chimp': 'D',
            'Chimp128': 'p',
            'Patas': '+',
            'PDE': 'X',
            'ALP': '*',
            'Gorilla': 's',
            'ELF': 'd',
            'Zstd': 'O'
        }

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
            palette=colors,
            alpha=1
        )  

        ax1.xaxis.grid(
            linewidth=0.5,
            color='#ededed',
            #alpha=0.2
        )

        ax1.yaxis.grid(
            linewidth=0.5,
            color='#ededed',
            #alpha=0.2
        )

        ax1.set_xlabel('Compression Speed\nas Tuples per CPU Cycle (Log Scale)', fontdict={"size": 8})
        ax1.set_ylabel('Decompression Speed\nas Tuples per CPU Cycle (Log Scale)', fontdict={"size": 8})

        ax1.set_yscale('log')
        ax1.set_xscale('log')

        ax1.set_axisbelow(True)

        ax1.text(
            0.05, 0.01,
            "1.7x", #"38.1",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['Chimp']
        )

        ax1.text(
            0.12, 0.03,
            "2.2x", #"29.3",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['Chimp128']
        )

        ax1.text(
            0.07, 0.40,
            "1.8x", #"36.4",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['Patas']
        )

        ax1.text(
            0.003, 0.14,
            "2.0x",#"33.1",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['PDE']
        )

        ax1.text(
            0.04, 0.06,
            "1.5x",#"33.1",
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['Gorilla']
        )

        ax1.text(
            0.004, 0.01,
            "2.8x", #
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['ELF']
        )

        ax1.text(
            0.011, 0.09,
            "3.1x", #
            fontsize=8,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['Zstd']
        )

        ax1.text(
            0.35, 2.5,
            "Compression Ratio:\n3.0x", #"Compression Ratio: 23.7\nBits per Value",
            fontsize=8.5,
            horizontalalignment='right',
            fontweight='bold',
            color=colors['ALP']
        )


        handles, labels = ax1.get_legend_handles_labels()
        #  ['Patas', 'Chimp', 'Chimp128', 'PDE', 'ALP', 'Gorilla'])
        handles, labels
        order = [4,3,0,2,1,5]
        #ax.legend()

        ax1.legend(
        #     [handles[idx] for idx in order],[labels[idx] for idx in order],
            loc="upper left",
            prop={'size': 6.5},
            frameon=False,
            ncols=2
        )

        ax1.set_ylim((0, 13))

        plt.savefig(f'{self.out_directory}/Speed.eps', format='eps', dpi=800, bbox_inches='tight')
        plt.savefig(f'{self.out_directory}/Speed.png', format='png', dpi=800, bbox_inches='tight')


    def plot_fused_unfused(self):
        df_bw = pd.read_csv(f'{self.results_directory}/i4i/x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')
        df_bw = df_bw[df_bw['name'].str.contains('bw')]
        df_bw['process'] = df_bw['name'].apply(self.get_fused_process)
        df_bw['tuples_per_cycle'] = 1 / df_bw['cycles_per_tuple']
        df_bw = df_bw[['tuples_per_cycle', 'process', 'benchmark_number']]
        
        df = pd.read_csv(f'{self.results_directory}/i4i/x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')
        df = df[~df['name'].str.contains('bw')]
        df['tuples_per_cycle'] = 1 / df['cycles_per_tuple']
        df['process'] = df['name'].apply(self.get_fused_process)
        # df = df[
        #     ~df['name'].str.contains('gov')
        # ]
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

        colors = {
            'Fused': '#4b8bf5',
            'Non-Fused': '#f6b26b'
        }

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
            palette=colors
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
            palette=colors
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
            palette=colors,
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
            palette=colors
        )

        sns.lineplot(
            data=df_bw[0:-24], 
            x="benchmark_number", 
            y="tuples_per_cycle", 
            hue='process',
            ax=ax2d,
            linewidth=0.6,
            markers=False,
            palette=colors,
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
            palette=colors
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
        plt.savefig(f'{self.out_directory}/Fusing.eps', format='eps', dpi=800, bbox_inches='tight')
        plt.savefig(f'{self.out_directory}/Fusing.png', format='png', dpi=800, bbox_inches='tight')


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
            pd.read_csv(f'{self.results_directory}/i4i/fallback_scalar_aav_1024_uf1_falp.csv'),   # Auto-Vectorized
            pd.read_csv(f'{self.results_directory}/i4i/fallback_scalar_nav_1024_uf1_falp.csv'),   # Scalar
            pd.read_csv(f'{self.results_directory}/i4i/x86_64_avx512bw_intrinsic_1024_uf1_falp.csv')# SIMDized
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

        colors = {
            'Auto-Vectorized': '#e06666',
            'Scalar': '#f6b26b',
            'SIMDized': '#4b8bf5'
        }

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
            palette=colors
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

        plt.savefig(f'{self.out_directory}/Architectures.eps', format='eps', dpi=800, bbox_inches='tight')
        plt.savefig(f'{self.out_directory}/Architectures.png', format='png', dpi=800, bbox_inches='tight')