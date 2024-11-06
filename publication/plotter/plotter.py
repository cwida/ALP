from plot_maker import PlotMaker
import warnings
warnings.filterwarnings("ignore", message="findfont: Font family 'Droid Serif' not found")
        
if __name__ == "__main__":
    pm = PlotMaker()
    pm.plot_architectures() # Figure 4
    pm.plot_fused_unfused() # Figure 5
    pm.plot_speed() # Figure 1 

    # TODO: Figure 6
