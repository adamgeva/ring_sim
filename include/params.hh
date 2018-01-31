#ifndef PARAMS_HH_
#define PARAMS_HH_
//comment for a single threaded mode
#define MULTI 1
#define NUM_OF_SCORERS 5
//ALT_SOURCES 1: all NUM_OF_SOURCES will be used, ALT_SOURCES 0: only 1 source is used
#define ALT_SOURCES 1
#define NUM_OF_SOURCES 100 //this defines the number of runs
#define NUM_OF_SPECTRUM_BINS 150 //from spectrum MATLAB plot
#define NUM_OF_VOXELS 784 //this defines the number of voxels - redundant
#define NUM_OF_ELEMENTS 5 //this defines the number of elements
#define NUM_OF_THREADS 30
#define NUM_OF_MATERIALS 100 // all materials are quantized to 100 materials with different densities
#define NUM_OF_PHOTONS 100000 // equal to mac file
// verbose
#define VERBOSE_SCORING 0
#define VERBOSE_PHYSICS_LIST 0
#define VERBOSE_ANALYSIS_MANAGER 0
#define VERBOSE_VIS 0
#define VERBOSE_RUN 1
#define VERBOSE_EVENT 0
#define VERBOSE_TRACK 0
#define VERBOSE_G4NAVIGATOR 0
#define BIASING 0 //set to 1 for on biasing
#define SPLITTING_FACTOR_NP 70
#define SPLITTING_FACTOR_NS 70
#define DETECTOR_SPECIAL_CUTS 1
#define PHANTOM_PRODUCTION_CUTS 0
#define KILL_ELECTRONS 0
#define RECORD_HIST 0 //when 0 - no histograms will be recorded in the simulation
#define PRINT_ELEMENTS_XS 0
//Particle Gun
#define PARTICLE_ENERGY 150 //KeV
#define MIN_THETA 0
#define MAX_THETA 0.5236 
#define MIN_PHI 0
#define MAX_PHI 0
//Geometry
#define BUILD_DETECTORS 1
#define BUILD_PHANTOM 1
#define WORLD_XY 210 //cm half size
#define WORLD_Z 210 //cm half size
#define WATER_BOX 5 //cm
#define PHANTOM_XY (0.1 * WORLD_XY)
#define PHANTOM_Z (0.1 * WORLD_Z)
#define DETECTOR_X 15 //mm half size
#define DETECTOR_Y 30 //mm half size. 2mm * numberOfRows
#define DETECTOR_Z 0.8 //mm half size. detector depth
#define RADIUS 45 //cm
#define SHIFT 1 //cm
#define NUM_OF_ROWS 1
#define GT_MODE 0
//XCAT
#define NUM_OF_Z_SLICES 1
#define NUM_OF_VOXELS_X 28 
#define NUM_OF_VOXELS_Y 28 
#define NUM_OF_PIXELS_SLICE (NUM_OF_VOXELS_X * NUM_OF_VOXELS_Y)
#define VOXEL_HALF_X 0.45 //cm
#define VOXEL_HALF_Y 0.45 //cm
#define VOXEL_HALF_Z 4.5 //cm
#define FILE_SPECTRUM "../run_inputs/spectrum.txt"
#define FILE_MATERIALS "../run_inputs/materials.txt"
#define FILE_VOXEL_TO_MATERIALS "../run_inputs/voxels_to_materials.txt"
#define FILE_FIXED_SOURCE "../run_inputs/all_sources.txt"
#define FILE_SOURCE_TO_DET "../run_outputs_geom/sourceToDet.csv"
#define FILE_SOURCE_POS "../run_outputs_geom/sourcesPos.csv"
#define FILE_DET_POS "../run_outputs_geom/detectorsPos.csv"
#define OUTPUT_DIR "../run_outputs"
#endif /* PARAMS_HH_ */
