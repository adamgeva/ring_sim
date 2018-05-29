
#include "B1RunAction.hh"
#include "G4EmCalculator.hh"
#include "G4NistManager.hh"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

B1RunAction::B1RunAction(B1DetectorConstruction* detectorConstruction)
 : G4UserRunAction(),
   fGradientAccumulable("grad_accum"),fdetectorConstruction(detectorConstruction),farr_error(0)
{
	if (CALC_GRADIENT == 1){
		// Register accumulable to the accumulable manager
		G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
		accumulableManager->RegisterAccumulable(&fGradientAccumulable);
	}
}
B1RunAction::~B1RunAction()
{}

G4Run* B1RunAction::GenerateRun()
{
	return new B1Run;

}

void B1RunAction::BeginOfRunAction(const G4Run* run)
{
	G4int runID = run->GetRunID();

	//rotate phantom
	if (BUILD_PHANTOM == 1){

		G4String fileName = std::string(OUTPUT_DIR) + "/angles/" + IntToString(runID) + ".csv";
		G4int randomized_num;
		G4double angle = M_PI/90; //2 degrees
		if (CALC_GRADIENT == 0){
			if (ROTATE_SEQUENTIALLY == 1){
				fdetectorConstruction->setContainerRotation(angle * runID);
			}else{
				//if we are running in forward mode: we sample and write to files:
				//sample # of directions randomly:
				srand (time(NULL));
				randomized_num = rand()%180;
				std::ofstream output;
				output.open(fileName.c_str());
				//write #:
				output << randomized_num << '\n';
				output.close();
				fdetectorConstruction->setContainerRotation(angle * randomized_num);
				//fdetectorConstruction->setContainerRotation(angle * runID);

			}
		}
		else{
			//if we are in inverse mode we read the files:
			fileName = std::string(INPUT_DIR) + "/angles/" + IntToString(runID) + ".csv";
			std::ifstream fin(fileName.c_str(), std::ios_base::in);
			if( !fin.is_open() ) {
			   G4Exception("Can't read angle file",
							"",
							FatalErrorInArgument,
							G4String("File not found " + fileName ).c_str());
			}
			fin >> randomized_num;
			fdetectorConstruction->setContainerRotation(angle * randomized_num);
		}

	}
	if (CALC_GRADIENT==1){
		//collect error from previous run:
		farr_error = new G4double[NUM_OF_DET_ROWS * NUM_OF_DET_COLS];
		std::string fname = std::string(ERROR_DIR) + "/error_" + IntToString(runID) + ".csv";
		std::ifstream fin(fname.c_str(), std::ios_base::in);
		if( !fin.is_open() ) {
		   G4Exception("Can't read error file",
						"",
						FatalErrorInArgument,
						G4String("File not found " + fname ).c_str());
		  }

		for (int row=0; row<NUM_OF_DET_ROWS; row++){
		  for (int col=0; col<NUM_OF_DET_COLS; col++) {
			  G4double err;
			  fin >> err;
			  farr_error[row * NUM_OF_DET_COLS + col] = err;
		  }
		}
		fin.close();

		// reset accumulables to their initial values
		G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
		accumulableManager->Reset();
		fGradientAccumulable.SetErrorArray(farr_error);
	}


}

void B1RunAction::EndOfRunAction(const G4Run* aRun)
{
	//write gradient table
	if (CALC_GRADIENT == 1){
		// Merge accumulables
		G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
		accumulableManager->Merge();

		//G4int threadID = G4Threading::G4GetThreadId();
		G4int runID = aRun->GetRunID();
		if(IsMaster()) {
			fGradientAccumulable.writeGradientAndP(runID);
		}

		delete  farr_error;
	}

	const B1Run* theRun = (const B1Run*)aRun;
	//writing to CSV file the cylinder response
	G4int numOfItr = NUM_OF_DET_COLS; //numOfItr holds the number of columns

	G4int runID = theRun->GetRunID();

	if (CALC_GRADIENT == 0){ //if we calculate gradient there is no need to plot I
		if(IsMaster()) {
			std::ofstream output;
			G4int total_scorers;
			if (EXTRA_SCORERS == 1){
				total_scorers = NUM_OF_SCORERS + NUM_EXTRA_SCORERS;
			} else {
				total_scorers = NUM_OF_SCORERS;
			}
			for (G4int k=0; k<total_scorers; k++){
				std::string fileName;
				if (GT_MODE == 1){
					fileName = std::string(OUTPUT_DIR) + "/I_test_LIV/run_" + IntToString(runID) + "outputEnergyDep_" + IntToString(k) + ".csv";
					//fileName = std::string(OUTPUT_DIR) + "/Flat/" + IntToString(runID) + "outputEnergyDep_" + IntToString(k) + ".csv";
				} else {

					fileName = std::string(OUTPUT_DIR) + "/I/run_" + IntToString(runID) + "outputEnergyDep_" + IntToString(k) + ".csv";
				}
				output.open(fileName.c_str());
				output << "OutPut Energy Deposit - source location, parameters" << "\n";
				//write response
				for (int i=0;i<numOfItr;i++)
				  {
					  for (int j=0; j<NUM_OF_DET_ROWS;j++)
					  {
						  G4double* eDep = (theRun->fMapSum[k])[i*NUM_OF_DET_ROWS +j];
						  if (eDep==0){
							  output<<"0,";
						  }
						  else {
							  output<< *eDep << ",";
							  //G4cout <<"response: "<< i*NUMBER_OF_ROWS +j<< " : "<< *eDep <<G4endl;
						  }
					  }
					  output<<"\n";
				  }
				output.close();
			}
		}
	}

//print fMaterials
	std::ofstream output;
	std::string fileName = "materails_Y.csv";

	output.open(fileName.c_str());


    for(std::vector<G4Material*>::iterator it = fdetectorConstruction->fMaterials.begin(); it != fdetectorConstruction->fMaterials.end(); ++it) {
    	G4Material* mat = *it;
    	G4String name = mat->GetName();

    	// std::cout << *it; ...
    	G4double den_t = (mat->GetDensity()) / (g/cm3);
    	G4EmCalculator emCalculator;
    	G4double Atten = emCalculator.ComputeGammaAttenuationLength(60*keV,mat);
    	output << Atten << "," << den_t << "\n";
//
//    	const G4ElementVector* curr_element_vector = mat->GetElementVector();
//		const G4double* curr_frac_vector = mat->GetFractionVector();
//		//vector of number of atoms per volume
//
//		G4int nElements = mat->GetNumberOfElements();
//		//incase there is compton scatter - calc dsigma:
//
//		for (G4int i=0 ; i<nElements ; i++) {
//				G4double frac_i = curr_frac_vector[i];
//				G4double dens_i = frac_i * den_t;
//				G4Element* el_i =  (*curr_element_vector)[i];
//				G4String el_Z = el_i->GetZ();
//				output << el_Z << "," << dens_i << ",";
//		}
//		output << "\n";

    }
	output.close();

//Only relevant when we need to print the GT
/*
	std::ofstream output_2;
	std::string fileName_2 = "materails_Y_ID.csv";
	output_2.open(fileName_2.c_str());

	//print voxel ID's
	G4int ID;
	G4int num_of_voxels = NUM_OF_VOXELS_X*NUM_OF_VOXELS_Y*NUM_OF_Z_SLICES;
	for (G4int vox=0; vox<num_of_voxels; vox++){
		ID = fdetectorConstruction->fMateIDs[vox];
		output_2 << ID << "\n";
	}
	output_2.close();
*/
}
