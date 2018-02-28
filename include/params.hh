#ifndef PARAMS_HH_
#define PARAMS_HH_
//comment for a single threaded mode
#define MULTI 1
#define NUM_OF_SCORERS 5
//ALT_SOURCES 1: all NUM_OF_SOURCES will be used, ALT_SOURCES 0: only 1 source is used
#define ALT_SOURCES 1
#define NUM_OF_SOURCES 100 //this defines the number of runs
#define NUM_OF_SPECTRUM_BINS 150 //from spectrum MATLAB plot
#define NUM_OF_ELEMENTS 5 //this defines the number of elements
#define NUM_OF_THREADS 30
#define NUM_OF_BASE_MATERIALS 9 // all materials are quantized to 100 materials with different densities
#define NUM_OF_PHOTONS 100000000 // equal to mac file
#define NUM_OF_DETECTORS 94
// verbose
#define VERBOSE_SCORING 0
#define VERBOSE_PHYSICS_LIST 0
#define VERBOSE_ANALYSIS_MANAGER 0
#define VERBOSE_VIS 0
#define VERBOSE_RUN 1
#define VERBOSE_EVENT 0
#define VERBOSE_TRACK 0
#define VERBOSE_G4NAVIGATOR 0
#define PRINT_PATHS 0
#define BIASING 0 //set to 1 for on biasing
#define SPLITTING_FACTOR_NP 70
#define SPLITTING_FACTOR_NS 70
#define DETECTOR_SPECIAL_CUTS 1
#define PHANTOM_PRODUCTION_CUTS 0
#define KILL_ELECTRONS 1
#define RECORD_HIST 0 //when 0 - no histograms will be recorded in the simulation
#define PRINT_ELEMENTS_XS 0
//Particle Gun
#define PARTICLE_ENERGY 150 //KeV
#define MIN_THETA 0
#define MAX_THETA M_PI/14
#define MIN_PHI 0
#define MAX_PHI (2 * M_PI)
//Geometry
#define BUILD_DETECTORS 1
#define BUILD_PHANTOM 1
#define CT_PHANTOM 1 //build phantom using linear reconstruction
#define CALC_GRADIENT 1
#define WORLD_XY 210 //cm half size
#define WORLD_Z 210 //cm half size
#define WATER_BOX 5 //cm
#define PHANTOM_XY (0.1 * WORLD_XY)
#define PHANTOM_Z (0.1 * WORLD_Z)
#define DETECTOR_X (0.278/2) //mm half size
#define DETECTOR_Y (0.278/2 * NUM_OF_DET_ROWS) //mm half size. 2mm * numberOfRows
#define DETECTOR_Z 0.8 //mm half size. detector depth
#define NUM_OF_DET_COLS 1088
#define NUM_OF_DET_ROWS 896
#define CENTER_TO_DET 267.38 //mm
#define SOURCE_TO_CENTER 605.27 //mm
#define RADIUS 605.27 //mm

#define SHIFT 1 //cm
#define GT_MODE 0
//XCAT
#define NUM_OF_Z_SLICES 400
#define NUM_OF_VOXELS_X 512
#define NUM_OF_VOXELS_Y 512
#define NUM_OF_PIXELS_SLICE (NUM_OF_VOXELS_X * NUM_OF_VOXELS_Y)
#define NUM_OF_VOXELS (NUM_OF_PIXELS_SLICE * NUM_OF_Z_SLICES) //this defines the number of voxels - redundant
#define VOXEL_HALF_X (0.045/2) //cm
#define VOXEL_HALF_Y (0.045/2) //cm
#define VOXEL_HALF_Z (0.045/2) //cm
#define FILE_SPECTRUM "../run_inputs/spectrum.txt"
#define FILE_MATERIALS "../run_inputs/materials.txt"

#define FILE_VOXEL_TO_MATERIALS "../run_inputs/voxels_to_materials.txt"
#define FILE_VOXEL_TO_MATERIALS_ID "../run_inputs/voxels_to_materials_id.txt"
#define FILE_VOXEL_TO_MATERIALS_HEAD "../run_inputs/head.dat"
#define FILE_VOXEL_TO_MATERIALS_TEST "../run_inputs/phantom/"
#define FILE_VOXEL_TO_MATERIALS_DENS "../run_inputs/voxels_to_materials_dens.txt"

#define FILE_FIXED_SOURCE "../run_inputs/all_sources.txt"
#define FILE_SOURCE_TO_DET "../run_outputs_geom/sourceToDet.csv"
#define FILE_SOURCE_POS "../run_outputs_geom/sourcesPos.csv"
#define FILE_DET_POS "../run_outputs_geom/detectorsPos.csv"
#define OUTPUT_DIR "../run_outputs"
#define GRADIENT_DIR "../run_outputs_grad"
#endif /* PARAMS_HH_ */
