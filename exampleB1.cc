
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

#include "G4GenericBiasingPhysics.hh"
#include "G4UImanager.hh"
#include "G4ScoringManager.hh"
#include "QBBC.hh"
#include "FTFP_BERT.hh"
#include "G4PhysListFactory.hh"
#include "G4EmPenelopePhysics.hh"

#include "B1ExtraPhysics.hh"

#include "G4Run.hh"

#include "G4NistManager.hh"


#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include <iostream>

//calculate MFP
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4LivermoreComptonModel.hh"
#include "G4ComptonScattering.hh"
#include "G4MaterialCutsCouple.hh"
#include "G4PEEffectFluoModel.hh"

#include "G4StepLimiterPhysics.hh"
#include "B1ModularPhysicsList.hh"

void printXSPerAtom(G4Element* el); //implemented at the end

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
	G4VModularPhysicsList* physicsList;
	if (parameters.Bias.phantomProductionCuts == true){
		physicsList = new B1ModularPhysicsList("myList");
	}
	else{
		G4PhysListFactory factory;
		physicsList = factory.GetReferencePhysList("FTFP_BERT_EMV");
		//FTFP_BERT_PEN
		//G4VModularPhysicsList* physicsList = new QBBC;
	}

	//physicsList->SetCutsForRegion(10*km,"waterRegion");

	//biasing
	G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();
	  if ( parameters.Myparams.onOffBiasing == 1 )
	    {
	      //TODO:fix
		  biasingPhysics->Bias("gamma");
	      // -- Create list of physics processes to be biased: only brem. in this case:
	      //TODO: is this necessary??
	      //std::vector< G4String > processToBias;
	      //processToBias.push_back("compt");
	      // -- Pass the list to the G4GenericBiasingPhysics, which will wrap the eBrem
	      // -- process of e- and e+ to activate the biasing of it:
	      //biasingPhysics->PhysicsBias("gamma", processToBias);

	      physicsList->RegisterPhysics(biasingPhysics);
	      G4cout << "      ********************************************************* " << G4endl;
	      G4cout << "      ********** processes are wrapped for biasing ************ " << G4endl;
	      G4cout << "      ********************************************************* " << G4endl;
	    }
	  else
	    {
	      G4cout << "      ************************************************* " << G4endl;
	      G4cout << "      ********** processes are not wrapped ************ " << G4endl;
	      G4cout << "      ************************************************* " << G4endl;
	    }

	physicsList->SetVerboseLevel(parameters.Myparams.physicsListVerbose);

	if (parameters.Bias.detectorSpecialCuts==true) {
	physicsList->RegisterPhysics(new B1ExtraPhysics());
	}
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

	//******************************************************************
	//print elements cross section
	if (parameters.Myparams.printElementsXS==true){
		printXSPerAtom(new G4Element("Hydrogen","H",1.0,1.008 * g/mole));
		printXSPerAtom(new G4Element("Helium","He",2.0,4.0026 * g/mole ));
		printXSPerAtom(new G4Element( "Lithium","Li",3.0, 6.941  * g/mole ));
		printXSPerAtom(new G4Element("Beryllium","Be",4.0, 9.012182  * g/mole ));
		printXSPerAtom(new G4Element("Boron","B",5.0, 10.811  * g/mole ));
		printXSPerAtom(new G4Element( "Carbon","C",6.0, 12.011 * g/mole ));
		printXSPerAtom(new G4Element( "Nitrogen","N",7.0, 14.007 * g/mole ));
		printXSPerAtom(new G4Element( "Oxygen","O",8.0, 16.00  * g/mole ));
		printXSPerAtom(new G4Element( "Fluorine","F",9.0, 18.998404  * g/mole ));
		printXSPerAtom(new G4Element(  "Neon","Ne",10.0, 20.1797  * g/mole ));
		printXSPerAtom(new G4Element( "Sodium","Na",11.0, 22.98977 * g/mole ));
		printXSPerAtom(new G4Element( "Magnesium","Mg",12.0, 24.305 * g/mole ));
		printXSPerAtom(new G4Element( "Aluminum","Al",13.0, 26.981539 * g/mole ));
		printXSPerAtom(new G4Element( "Phosphorus","P",15.0, 30.97376 * g/mole ));
		printXSPerAtom(new G4Element( "Sulfur","S",16.0,32.065* g/mole ));
		printXSPerAtom(new G4Element( "Chlorine","Cl",17.0, 35.453* g/mole ));
		printXSPerAtom(new G4Element( "Argon","Ar",18.0, 39.948 * g/mole ));
		printXSPerAtom(new G4Element( "Potassium","K",19.0, 39.0983* g/mole ));
		printXSPerAtom(new G4Element("Calcium","Ca",20.0, 40.078* g/mole ));
		printXSPerAtom(new G4Element( "Scandium","Sc",21.0, 44.95591 * g/mole ));
		printXSPerAtom(new G4Element( "Titanium","Ti",22.0, 47.867 * g/mole ));
		printXSPerAtom(new G4Element( "Vanadium","V",23.0, 50.9415 * g/mole ));
		printXSPerAtom(new G4Element( "Chromium","Cr",24.0, 51.9961 * g/mole ));
		printXSPerAtom(new G4Element( "Manganese","Mn",25.0, 54.93805 * g/mole ));
		printXSPerAtom(new G4Element( "Iron","Fe",26, 55.845* g/mole ));
		printXSPerAtom(new G4Element( "Iodine","I",53, 126.90447 * g/mole ));
		printXSPerAtom(new G4Element( "Lead","Pb",82, 207.2 * g/mole ));
	}

	//**********************************************************************

	// Job termination
	// Free the store: user actions, physics_list and detector_description are
	// owned and deleted by the run manager, so they should not be deleted
	// in the main() program !

	delete visManager;
	delete runManager;
}


void printXSPerAtom(G4Element* el){
	params parameters;
	G4EmCalculator emCalculator;
	std::string elementName = el->GetName();
	G4int elementZ = el->GetZ();
	std::ofstream outputElementFile;

	std::string fileName =  "../run_outputs/ElementsXS/" + IntToString(elementZ) + ".csv";
	outputElementFile.open(fileName.c_str());

	G4double highE = parameters.MyparamsGun.particleEnergy + 3*keV; //3 is buffer
	G4double DeltaEnergy = 0.1*keV; //delta Energy
	G4double Energy = DeltaEnergy;

	G4double comptXS;
	G4double photXS;

	// print phot
	while (Energy<highE) {
		photXS = emCalculator.ComputeCrossSectionPerAtom(Energy,"gamma","phot",el,0);
		outputElementFile << photXS/cm2 << ",";
		Energy = Energy + DeltaEnergy;
	}
	outputElementFile << "\n";
	// print compt
	Energy = DeltaEnergy; //delta Energy
	while (Energy<highE) {
		comptXS = emCalculator.ComputeCrossSectionPerAtom(Energy,"gamma","compt",el,0);
		outputElementFile << comptXS/cm2 << ",";
		Energy = Energy + DeltaEnergy;
	}
	outputElementFile << "\n";
	outputElementFile.close();

}
