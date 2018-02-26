
#include "params.hh"
#include "globalFunctions.hh"
#include "B1RunAction.hh"
#include "Analysis.hh"
#include "B1Run.hh"
#include "G4AccumulableManager.hh"
#include "G4Accumulable.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

extern B1EnergyDeposit* detectorsArray[NUM_OF_THREADS];


B1RunAction::B1RunAction(B1DetectorConstruction* detectorConstruction)
 : G4UserRunAction()
   //fGradientAccumulable("grad_accum"),fdetectorConstruction(detectorConstruction)
{
	fMyEnergyDeposit = 0;
	// Register accumulable to the accumulable manager
	//G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
	//accumulableManager->RegisterAccumulable(&fGradientAccumulable);
}
B1RunAction::~B1RunAction()
{}

G4Run* B1RunAction::GenerateRun()
{
	G4int Ind;
	G4int threadID = G4Threading::G4GetThreadId();
	if (threadID ==-1){
		Ind = 0;
	}
	else
	{
		Ind = threadID+1;
	}
	fMyEnergyDeposit = detectorsArray[Ind];
	return new B1Run;

}

void B1RunAction::BeginOfRunAction(const G4Run* run)
{
	G4int runID = run->GetRunID();

	if (PRINT_PATHS == 1){
		G4int threadID = G4Threading::G4GetThreadId();
		fMyEnergyDeposit->openFile(threadID,runID);
	}

	//rotate phantom
	if (BUILD_PHANTOM == 1){
		//G4double angle = M_PI/90; //2 degrees
		//fdetectorConstruction->setContainerRotation(angle * runID);
	}
	// reset accumulables to their initial values
	//G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
	//accumulableManager->Reset();

}

void B1RunAction::EndOfRunAction(const G4Run* aRun)
{

	if (PRINT_PATHS == 1){
		//write EnergyDepositFile
		fMyEnergyDeposit->writeFile();
	}

	// Merge accumulables
	//G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
	//accumulableManager->Merge();


	//write gradient table
	if (CALC_GRADIENT == 1){
		//G4int threadID = G4Threading::G4GetThreadId();
		G4int runID = aRun->GetRunID();
		if(IsMaster()) {
			//fGradientAccumulable.writeGradientAndP(runID);
		}
	}

	const B1Run* theRun = (const B1Run*)aRun;
	//writing to CSV file the cylinder response
	G4int numOfItr = NUM_OF_DET_COLS; //numOfItr holds the number of columns

	G4int runID = theRun->GetRunID();

	if(IsMaster()) {
		std::ofstream output;
		for (G4int k=0; k<NUM_OF_SCORERS; k++){
			std::string fileName;
			if (GT_MODE == 1){
				fileName = std::string(OUTPUT_DIR) + "/I_test_LIV/run_" + IntToString(runID) + "outputEnergyDep_" + IntToString(k) + ".csv";
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
