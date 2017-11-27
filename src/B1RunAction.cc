
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


B1RunAction::B1RunAction()
 : G4UserRunAction()
{}
B1RunAction::~B1RunAction()
{}

G4Run* B1RunAction::GenerateRun()
{ return new B1Run; }

void B1RunAction::BeginOfRunAction(const G4Run* /*run*/)
{}

void B1RunAction::EndOfRunAction(const G4Run* aRun)
{
	params parameters;
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
