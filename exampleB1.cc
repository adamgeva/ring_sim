
#include "B1DetectorConstruction.hh"
#include "B1ActionInitialization.hh"
#include "params.hh"
#include "globalFunctions.hh"

//TODO: how to control G4MULTITHREADED in a generic way???
#ifdef MULTI
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "G4ScoringManager.hh"
#include "QBBC.hh"
#include "FTFP_BERT.hh"
#include "G4PhysListFactory.hh"
#include "G4EmPenelopePhysics.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include <iostream>


int main(int argc,char** argv)
{
	params parameters;
	// Detect interactive mode (if no arguments) and define UI session
	G4UIExecutive* ui = 0;
	if ( argc == 1 ) {
		ui = new G4UIExecutive(argc, argv);
	}

	// Choose the Random engine
	CLHEP::RanecuEngine* eng = new CLHEP::RanecuEngine;
	//eng->setIndex(8);
	G4Random::setTheEngine(eng);

	// Construct the default run manager
	#ifdef MULTI
		  G4MTRunManager* runManager = new G4MTRunManager;
		  runManager->SetNumberOfThreads(parameters.Myparams.numberOfThreads);
		  std::cout << "numOfThreads: " << runManager-> GetNumberOfThreads() << std::endl;
	#else
		  G4RunManager* runManager = new G4RunManager;
		  std::cout << "Single-Threaded Mode" <<  std::endl;
	#endif


	// Activate UI-command base scorer
	G4ScoringManager * scManager = G4ScoringManager::GetScoringManager();
	scManager->SetVerboseLevel(parameters.Myparams.scoringVerbose);

	// Set mandatory initialization classes
	// Detector construction
	runManager->SetUserInitialization(new B1DetectorConstruction());

	// Physics list
	G4PhysListFactory factory;
	G4VModularPhysicsList* physicsList = factory.GetReferencePhysList("FTFP_BERT_PEN");
	//FTFP_BERT_PEN
	//G4VModularPhysicsList* physicsList = new FTFP_BERT;
	physicsList->SetVerboseLevel(parameters.Myparams.physicsListVerbose);
	runManager->SetUserInitialization(physicsList);

	// User action initialization
	runManager->SetUserInitialization(new B1ActionInitialization());

	// Initialize visualization
	G4VisManager* visManager = new G4VisExecutive;
	// G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
	// G4VisManager* visManager = new G4VisExecutive("Quiet");
	visManager->SetVerboseLevel(parameters.Myparams.visVerbose);
	visManager->Initialize();

	// Get the pointer to the User Interface manager
	G4UImanager* UImanager = G4UImanager::GetUIpointer();
	 UImanager->ApplyCommand("/run/verbose " + IntToString(parameters.Myparams.runVerbose));
	 UImanager->ApplyCommand("/event/verbose " + IntToString(parameters.Myparams.eventVerbose));
	 UImanager->ApplyCommand("/tracking/verbose " + IntToString(parameters.Myparams.trackVerbose));

	// Process macro or start UI session
	if ( ! ui ) {
		// batch mode
		G4String command = "/control/execute ";
		G4String fileName = argv[1];
		UImanager->ApplyCommand(command+fileName);
	}
	else {
		// interactive mode
		UImanager->ApplyCommand("/control/execute init_vis.mac");
		ui->SessionStart();
		delete ui;
	}

	// Job termination
	// Free the store: user actions, physics_list and detector_description are
	// owned and deleted by the run manager, so they should not be deleted
	// in the main() program !

	delete visManager;
	delete runManager;
}

