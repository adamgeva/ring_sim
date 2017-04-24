
#include "params.hh"
#include "globalFunctions.hh"
#include "B1RunAction.hh"
#include "Analysis.hh"


#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <string>
#include <sstream>


B1RunAction::B1RunAction()
 : G4UserRunAction()
{
	params parameters;
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
		if (parameters.Myparams.fastSim==0){
			analysisManager->CreateH3("hist" + histNum,"XY and Energy histogram of detector hits",parameters.MyparamsHist.nx,
					-parameters.MyparamsGeometry.detectorXY/cm,parameters.MyparamsGeometry.detectorXY/cm,parameters.MyparamsHist.ny,
					-parameters.MyparamsGeometry.detectorXY/cm,parameters.MyparamsGeometry.detectorXY/cm,parameters.MyparamsHist.nz,
					0,(parameters.MyparamsGun.particleEnergy+buff)/keV);

		}
		else{
			analysisManager->CreateH3("hist" + histNum,"XY and Energy histogram of detector hits",1,-7.5,7.5,1,-7.5,7.5,1,0,100);

		}
	}
	//G4cout << "H3 Created " <<  G4endl;

}

B1RunAction::~B1RunAction()
{
	delete G4AnalysisManager::Instance();
}


void B1RunAction::BeginOfRunAction(const G4Run* /*run*/)
{
	//inform the runManager to save random number seed
	//G4RunManager::GetRunManager()->SetRandomNumberStore(true);

	// Get analysis manager
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

	// Open output file at each new run
	// The default file name is set in RunAction::RunAction(),
	// it can be overwritten in a macro
	analysisManager->OpenFile();
}

void B1RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  // Write and close output file
  // save histograms
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();

}
