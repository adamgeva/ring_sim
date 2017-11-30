/*
 * params.hh
 *
 *  Created on: Apr 14, 2017
 *      Author: adamgeva
 */

#ifndef PARAMS_HH_
#define PARAMS_HH_
#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include <math.h>
#include "B1EnergyDeposit.hh"

//comment for a single threaded mode
#define MULTI 1

#define NUM_OF_SCORERS 5
//ALT_SOURCES 1: all NUM_OF_SOURCES will be used, ALT_SOURCES 0: only 1 source is used
#define ALT_SOURCES 1
#define NUM_OF_SOURCES 10 //this defines the number of runs
#define NUM_OF_VOXELS 9 //this defines the number of voxels - redundant
#define NUM_OF_ELEMENTS 27 //this defines the number of elements
#define NUM_OF_THREADS 20

class params
{
public:
	struct Myparams{
		//todo: fix
		G4int numberOfThreads = NUM_OF_THREADS;
		G4int scoringVerbose = 0;
		G4int physicsListVerbose = 0;
		G4int analysisManagerVerbose = 0;
		G4int visVerbose = 0;
		int runVerbose = 1;
		int eventVerbose = 0;
		int trackVerbose = 0;
		//set to 0 to suppress navigator msgs
		int G4navigatorVerbos = 0;
		G4int onOffBiasing = 0; //set to 1 for on biasing
		G4int recordHist = 0; //when 0 - no histograms will be recorded in the simulation
		G4bool printElementsXS = false;
		G4int numberOfElements = 27; //equal to the number of elements we print at the end of exampleB1
		G4int* RunId; //donot change
	}Myparams;

	struct MyparamsGun{
		G4double particleEnergy = 57*keV; //keV
		G4int detectorCoverage = 1;
		G4double MinTheta = 0 ;
		G4double MaxTheta = M_PI/6;
		//G4double MaxTheta = 0;
		G4double MinPhi = 0;
//		G4double MaxPhi = 2*M_PI;
		G4double MaxPhi = 0;
	}MyparamsGun;

	struct Bias{
		//G4int ComptSplittingFactor = 10;
		G4int splittingFactorNp = 70;
		G4int splittingFactorNs = 70;
		G4bool detectorSpecialCuts = true;
		G4bool phantomProductionCuts = false;
		G4bool killElectrons = false;

	}Bias;


	struct MyparamsGeometry{
		//flags to build phantom and detectors
		G4int buildDetectors = 1;
		G4int buildPhantom = 1;
		G4double worldXY = 70*cm; //half sizes
		G4double worldZ = 70*cm;
		G4double waterBox = 5*cm;
		G4double phantomXY = 0.1*worldXY;
		G4double phantomZ = 0.1*worldZ;
		G4double boneX = 0.02*worldXY;
		G4double boneY = 0.1*worldZ;
		G4double boneZ = 0.02*worldXY;
		G4double detectorX = 15*mm; //half size
		G4double detectorY = 30*mm; //2mm * numberOfRows
		//G4double detectorY = 2*mm; //2mm * numberOfRows
		G4double detectorZ = 0.8*mm; //detector depth
		G4double radius = 15*cm;
		G4double shift = 1*cm; //shift of each ring
		G4int numberOfRows = 1;
		//G4int numberOfRows = 1;
		G4int numberOfDetectors = 5;
		//XCAT
		//TODO: add in an external meta file
		G4int numberOfZSlices = 1;
		G4int numberOfVoxelsX = 3;
		G4int numberOfVoxelsY = 3;
		G4int numberOfPixelsPerSlice = numberOfVoxelsX*numberOfVoxelsY;
		//TODO: check that these sizes are correct
		G4double voxelHalfX = 1.5*cm;
		G4double voxelHalfY = 1.5*cm;
		G4double voxelHalfZ = 1.5*cm;
		std::string voxels_materials_file = "../run_inputs/voxels_materials.txt";

	}MyparamsGeometry;

};


#endif /* PARAMS_HH_ */
