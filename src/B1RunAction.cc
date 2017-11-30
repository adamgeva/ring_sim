
#include "params.hh"
#include "globalFunctions.hh"
#include "B1RunAction.hh"
#include "Analysis.hh"
#include "B1Run.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

extern B1EnergyDeposit* detectorsArray[NUM_OF_THREADS];


B1RunAction::B1RunAction()
 : G4UserRunAction()
{
	fMyEnergyDeposit = 0;
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
	G4int threadID = G4Threading::G4GetThreadId();
	G4int runID = run->GetRunID();
	fMyEnergyDeposit->openFile(threadID,runID);
}

void B1RunAction::EndOfRunAction(const G4Run* aRun)
{
	params parameters;

	//write EnergyDepositFile
	fMyEnergyDeposit->writeFile();

	const B1Run* theRun = (const B1Run*)aRun;
	//writing to CSV file the cylinder response
	G4double alpha = 2*atan(parameters.MyparamsGeometry.detectorX/parameters.MyparamsGeometry.radius);
	G4int numOfItr = (2*M_PI)/alpha; //numOfItr holds the number of columns

	G4int runID = theRun->GetRunID();

	if(IsMaster()) {
		std::ofstream output;
		for (G4int k=0; k<NUM_OF_SCORERS; k++){
			std::string fileName = "../run_outputs/run_" + IntToString(runID) + "outputEnergyDep_" + IntToString(k) + ".csv";
			output.open(fileName.c_str());
			output << "OutPut Energy Deposit - source location, parameters" << "\n";
			//write response
			for (int i=0;i<numOfItr;i++)
			  {
				  for (int j=0; j<parameters.MyparamsGeometry.numberOfRows;j++)
				  {
					  G4double* eDep = (theRun->fMapSum[k])[i*parameters.MyparamsGeometry.numberOfRows +j];
					  if (eDep==0){
						  output<<"0,";
					  }
					  else {
						  output<< *eDep << ",";
						  //G4cout <<"response: "<< i*parameters.MyparamsGeometry.numberOfRows +j<< " : "<< *eDep <<G4endl;
					  }
				  }
				  output<<"\n";
			  }
			output.close();
		}
	}
}
