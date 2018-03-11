
#include "B1RunAction.hh"

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
			for (G4int k=0; k<NUM_OF_SCORERS; k++){
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
}
