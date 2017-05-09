
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
{
	params parameters;
	//create analysis manager and histograms only if recordHist=1
	if (parameters.Myparams.recordHist==1){
		// Create analysis manager
		// The choice of analysis technology is done via selectin of a namespace
		// in Analysis.hh
		G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
		G4cout << "Using " << analysisManager->GetType() << G4endl;

		// Default settings
		analysisManager->SetVerboseLevel(parameters.Myparams.analysisManagerVerbose);
		analysisManager->SetFileName("OutputH3");
		for (int i=0;i<parameters.MyparamsGeometry.numberOfDetectors;i++){
			// Creating 3D histograms
			std::string histNum = IntToString(i);
			G4double buff=3*keV; //adding buffer to max value of hist energy
			analysisManager->CreateH3("hist" + histNum,"XY and Energy histogram of detector hits",parameters.MyparamsHist.nx,
					-parameters.MyparamsGeometry.detectorX/cm,parameters.MyparamsGeometry.detectorX/cm,parameters.MyparamsHist.ny,
					-parameters.MyparamsGeometry.detectorX/cm,parameters.MyparamsGeometry.detectorX/cm,parameters.MyparamsHist.nz,
					0,(parameters.MyparamsGun.particleEnergy+buff)/keV);
		}
		//G4cout << "H3 Created " <<  G4endl;
	}
}

B1RunAction::~B1RunAction()
{
	params parameters;
	//Analysis manager is created and deleted only if recordHist=1
	if (parameters.Myparams.recordHist==1){
		delete G4AnalysisManager::Instance();
	}
}

G4Run* B1RunAction::GenerateRun()
{ return new B1Run; }

void B1RunAction::BeginOfRunAction(const G4Run* /*run*/)
{
	params parameters;
	//inform the runManager to save random number seed
	//G4RunManager::GetRunManager()->SetRandomNumberStore(true);

	//Analysis manager is created and deleted only if recordHist=1
	if (parameters.Myparams.recordHist==1){
		// Get analysis manager
		G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
		// Open output file at each new run
		// The default file name is set in RunAction::RunAction(),
		// it can be overwritten in a macro
		analysisManager->OpenFile();
	}
}

void B1RunAction::EndOfRunAction(const G4Run* aRun)
{
	params parameters;
	//Analysis manager is created and Histograms are written only if recordHist=1
	if (parameters.Myparams.recordHist==1){
		// Write and close output file
		// save histograms
		G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
		analysisManager->Write();
		analysisManager->CloseFile();
	}
	const B1Run* theRun = (const B1Run*)aRun;
	//writing to CSV file the cylinder response
	G4double alpha = 2*atan(parameters.MyparamsGeometry.detectorX/parameters.MyparamsGeometry.radius);
	G4int numOfItr = (2*M_PI)/alpha; //numOfItr holds the number of columns

	if(IsMaster()) {
		std::ofstream output;
		for (G4int k=0; k<NUM_OF_SCORERS; k++){
			std::string fileName = "outputEnergyDep" + IntToString(k) + ".csv";
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
