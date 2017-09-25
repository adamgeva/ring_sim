/*
 * params.hh
 *
 *  Created on: Apr 14, 2017
 *      Author: adamgeva
 */

#ifndef PARAMS_HH_
#define PARAMS_HH_
#include "G4SystemOfUnits.hh"
#include "G4Types.hh"


//comment for a single threaded mode
#define MULTI 1

#define NUM_OF_SCORERS 5
//ALT_SOURCES 1: all NUM_OF_SOURCES will be used, ALT_SOURCES 0: only 1 source is used
#define ALT_SOURCES 1
#define NUM_OF_SOURCES 100 //this defines the number of runs



class params
{
public:
	struct Myparams{
		G4int numberOfThreads = 10;
		G4int scoringVerbose = 0;
		G4int physicsListVerbose = 0;
		G4int analysisManagerVerbose = 0;
		G4int visVerbose = 0;
		int runVerbose = 1;
		int eventVerbose = 0;
		int trackVerbose = 0;
		G4int onOffBiasing = 0; //set to 1 for on biasing
		G4int recordHist = 0; //when 0 - no histograms will be recorded in the simulation
	}Myparams;

	struct MyparamsGun{
		G4double particleEnergy = 57*keV; //keV
		G4int detectorCoverage = 1;
		G4double MinTheta = 0 ;
		G4double MaxTheta = M_PI/6;
		G4double MinPhi = 0;
//		G4double MaxPhi = 2*M_PI;
		G4double MaxPhi = 0;
	}MyparamsGun;

	struct Bias{
		//G4int ComptSplittingFactor = 10;
		G4int splittingFactorNp = 70;
		G4int splittingFactorNs = 70;
		G4bool BiasPrimaryOnly = true;
		G4bool BiasOnlyOnce = false;
		G4int BiasTimes = 4;

	}Bias;

	struct MyparamsGeometry{
		G4double worldXY = 70*cm; //half sizes
		G4double worldZ = 70*cm;
		G4double phantomXY = 0.1*worldXY;
		G4double phantomZ = 0.1*worldZ;
		G4double boneX = 0.02*worldXY;
		G4double boneY = 0.1*worldZ;
		G4double boneZ = 0.02*worldXY;
		G4double detectorX = 2*mm; //half size
		G4double detectorY = 128*mm; //2mm * numberOfRows
		//G4double detectorY = 2*mm; //2mm * numberOfRows
		G4double detectorZ = 0.8*mm; //detector depth
		G4double radius = 25*cm;
		G4double shift = 1*cm; //shift of each ring
		G4int numberOfRows = 64;
		//G4int numberOfRows = 1;
		G4int numberOfDetectors = 5;

	}MyparamsGeometry;

	struct MyparamsHist{
		G4int nx = 500;
		G4int ny = 500;
		G4int nz = 10;
	}MyparamsHist;
};


#endif /* PARAMS_HH_ */
