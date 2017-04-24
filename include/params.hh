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
//#define MULTI 1

class params
{
public:
	struct Myparams{
		G4int numberOfThreads = 30;
		G4int scoringVerbose = 0;
		G4int physicsListVerbose = 0;
		G4int analysisManagerVerbose = 0;
		G4int visVerbose = 0;
		int runVerbose = 1;
		int eventVerbose = 0;
		int trackVerbose = 0;
		G4int fastSim = 1; //1 for fast = no hits will be collected and the histograms are empty
		//Todo: add with and without mesh instead of fastSim
	}Myparams;

	struct MyparamsGun{
		G4double particleEnergy = 57*keV; //keV
		G4int detectorCoverage = 1;
	}MyparamsGun;

	struct MyparamsGeometry{
		G4double worldXY = 50*cm; //half sizes
		G4double worldZ = 50*cm;
		G4double phantomXY = 0.1*worldXY;
		G4double phantomZ = 0.1*worldZ;
		G4double boneX = 0.02*worldXY;
		G4double boneY = 0.1*worldZ;
		G4double boneZ = 0.02*worldXY;
		G4double detectorXY = 0.15*worldXY;
		G4double detectorZ = 5*mm; //detector depth
		G4double radius = 0.4*worldXY;
		G4int numberOfDetectors = 5;
	}MyparamsGeometry;

	struct MyparamsHist{
		G4int nx = 500;
		G4int ny = 500;
		G4int nz = 10;
	}MyparamsHist;
};


#endif /* PARAMS_HH_ */
