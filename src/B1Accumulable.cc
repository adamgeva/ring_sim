/*
 * B1Accumulable.cc
 *
 *  Created on: Feb 20, 2018
 *      Author: adamgeva
 */



#include "B1Accumulable.hh"
#include "G4VAccumulable.hh"
#include "params.hh"

#include "globalFunctions.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include "G4ios.hh"
#include "globals.hh"



B1Accumulable::B1Accumulable(const G4String& name):
G4VAccumulable(name)
{}

B1Accumulable:: ~B1Accumulable() {}

void B1Accumulable::updateP(G4int voxel_ind){
	//update P array:
	fP[voxel_ind]++;
}

void B1Accumulable::updateSm_hat(G4int voxel, G4int element, G4int detector, G4double value){
	//update gradient
	fSm_hat[voxel][element][detector] = fSm_hat[voxel][element][detector] + value;

}

void B1Accumulable::Merge(const G4VAccumulable& other){
	const B1Accumulable& otherAccumulable
	= static_cast<const B1Accumulable&>(other);

	//iterate and accumulate
	for (int i=0; i<NUM_OF_VOXELS; i++){
		fP[i] += otherAccumulable.fP[i];
		for (int j=0; j<NUM_OF_ELEMENTS; j++){
			for (int k =0; k<NUM_OF_DETECTORS; k++){
				fSm_hat[i][j][k] += otherAccumulable.fSm_hat[i][j][k];
			}
		}
	}
}

void B1Accumulable::Reset(){
	//iterate and reset arrays
	for (int i=0; i<NUM_OF_VOXELS; i++){
		fP[i] = 0;
		for (int j=0; j<NUM_OF_ELEMENTS; j++){
			for (int k =0; k<NUM_OF_DETECTORS; k++){
				fSm_hat[i][j][k] = 0;
			}
		}
	}
}

void B1Accumulable::writeGradientAndP(G4int runNum){
	//writing gradient table to file
	std::string fileName =  std::string(GRADIENT_DIR) + "/" + IntToString(runNum) + "run_gradient.csv";
	std::ofstream outputPathsFile_grad;
	outputPathsFile_grad.open(fileName.c_str());
	for (G4int element = 0; element < NUM_OF_ELEMENTS; element ++){
		for (G4int voxel = 0; voxel < NUM_OF_VOXELS; voxel ++){
			for (G4int det = 0; det < NUM_OF_DETECTORS; det ++){
				outputPathsFile_grad << fSm_hat[voxel][element][det] << ',';
			}
			outputPathsFile_grad << '\n';
		}
	}
	outputPathsFile_grad.close();

	//write P array:
	fileName =  std::string(GRADIENT_DIR) + "/" + IntToString(runNum) + "run_P.csv";
	std::ofstream outputPathsFile_P;
	outputPathsFile_P.open(fileName.c_str());
	for (G4int voxel = 0; voxel < NUM_OF_VOXELS; voxel ++){
		outputPathsFile_P <<  fP[voxel] << ',';
	}
	outputPathsFile_P.close();

}




