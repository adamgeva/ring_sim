#ifndef PARAMS_HH_
#define PARAMS_HH_
//comment for a single threaded mode
#define MULTI 1
#define NUM_OF_SCORERS 5
#define EXTRA_SCORERS 1
#define NUM_EXTRA_SCORERS 11
#define NUM_OF_SOURCES 180 //this defines the number of runs
#define NUM_OF_SPECTRUM_BINS 150 //from spectrum MATLAB plot
#define NUM_OF_ELEMENTS 6 //this defines the number of elements
#define NUM_OF_THREADS 20
#define NUM_OF_BASE_MATERIALS 10 // all materials are quantized to 100 materials with different densities
#define NUM_OF_PHOTONS 100000000 // equal to mac file
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
#define KILL_ELECTRONS 1
#define RECORD_HIST 0 //when 0 - no histograms will be recorded in the simulation
#define PRINT_ELEMENTS_XS 0
//Particle Gun
#define PARTICLE_ENERGY 60 //KeV
#define MIN_THETA 0
#define MAX_THETA M_PI/14 
#define MIN_PHI 0
#define MAX_PHI (2 * M_PI) 
//Geometry
#define BUILD_DETECTORS 1
#define BUILD_PHANTOM 1
#define XCAT 1//build XCAT phantom 
#define FILE_XCAT_ID_TO_COMP "../run_inputs/XCAT/id_to_comp.txt" 
#define FILE_XCAT_SLICE_PREFIX "../run_inputs/XCAT/hand/out_hand_act_" 
#define CT_PHANTOM 0 //build phantom using linear reconstruction 
#define USE_DICOM_INIT 0
#define ROTATE_SEQUENTIALLY 1
#define CALC_GRADIENT 0
#define WORLD_XY 210 //cm half size
#define WORLD_Z 210 //cm half size
#define DETECTOR_X 0.556 //mm half size
#define DETECTOR_Y 124.544 //mm half size. 
#define DETECTOR_Z 0.8 //mm half size. detector depth
#define NUM_OF_DET_COLS 272
#define NUM_OF_DET_ROWS 224
#define CENTER_TO_DET 267.38 //mm
#define SOURCE_TO_CENTER 605.27 //mm
#define RADIUS 605.27 //mm
#define OFFSET_U 0 //mm
#define OFFSET_V 0 //mm
#define SHIFT 1 //cm
#define GT_MODE 1
#define LIV_MODE 1
//XCAT
#define NUM_OF_Z_SLICES  80
#define NUM_OF_VOXELS_X 100 
#define NUM_OF_VOXELS_Y 100 
#define NUM_OF_VOXELS 800000 
#define NUM_OF_PIXELS_SLICE (NUM_OF_VOXELS_X * NUM_OF_VOXELS_Y)
#define VOXEL_HALF_X 1.15 //mm
#define VOXEL_HALF_Y 1.15 //mm
#define VOXEL_HALF_Z 1.15 //mm
#define FILE_SPECTRUM "../run_inputs/spectrum.txt"
#define FILE_MATERIALS "../run_inputs/materials.txt"
#define FILE_MATERIALS_DICOM_BASIC "../run_inputs/Dicom_base_materials_new_smart_init.txt"
#define FILE_VOXEL_TO_MATERIALS "../run_inputs/voxels_to_materials.txt"
#define FILE_VOXEL_TO_MATERIALS_ID "../run_inputs/voxels_to_materials_id.txt"
#define FILE_VOXEL_TO_MATERIALS_TEST "../run_inputs/phantom/"
#define FILE_VOXEL_TO_MATERIALS_Y "../run_inputs/Y/"
#define FILE_VOXEL_TO_MATERIALS_DENS "../run_inputs/voxels_to_materials_dens.txt"
#define FILE_FIXED_SOURCE "../run_inputs/all_sources.txt"
#define FILE_SOURCE_TO_DET "../run_outputs_geom/sourceToDet.csv"
#define FILE_SOURCE_POS "../run_outputs_geom/sourcesPos.csv"
#define FILE_DET_POS "../run_outputs_geom/detectorsPos.csv"
#define OUTPUT_DIR "../run_outputs"
#define OUTPUT_DIR_FLAT "../run_outputs/Flat"
#define INPUT_DIR "../run_inputs"
#define GRADIENT_DIR "../run_outputs_grad"
#define ERROR_DIR "../run_inputs/Error"
#endif /* PARAMS_HH_ */
