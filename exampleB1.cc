
#include "B1RegularDetectorConstruction.hh"
#include "B1ActionInitialization.hh"
#include "params.hh"
#include "globalFunctions.hh"
#include "G4SystemOfUnits.hh"



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
#include "B1EnergyDeposit.hh"


#include "G4Run.hh"

#include "G4NistManager.hh"


#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"

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
#include <ctime>
#include <iostream>


void printXSPerAtom(G4Element* el); //implemented at the end

//global array
B1EnergyDeposit* detectorsArray[NUM_OF_THREADS];

int main(int argc,char** argv)
{
	// Detect interactive mode (if no arguments) and define UI session
	G4UIExecutive* ui = 0;
	if ( argc == 1 ) {
		ui = new G4UIExecutive(argc, argv);
	}

	// Choose the Random engine
//	CLHEP::HepJamesRandom* eng = new CLHEP::HepJamesRandom;
//    std::time_t result = std::time(nullptr);
//    eng->setSeed(result);
//	G4Random::setTheEngine(eng);

	// Choose the Random engine
	CLHEP::RanecuEngine* eng = new CLHEP::RanecuEngine;
	std::time_t result = std::time(nullptr);
	eng->setSeed(result);
	//eng->setIndex(8);
	//eng->setSeed(8);
	G4Random::setTheEngine(eng);

	// Construct the default run manager
	#ifdef MULTI
		  G4MTRunManager* runManager = new G4MTRunManager;
		  runManager->SetNumberOfThreads(NUM_OF_THREADS);
		  std::cout << "numOfThreads: " << runManager-> GetNumberOfThreads() << std::endl;
	#else
		  G4RunManager* runManager = new G4RunManager;
		  std::cout << "Single-Threaded Mode" <<  std::endl;
	#endif


	// Activate UI-command base scorer
	G4ScoringManager * scManager = G4ScoringManager::GetScoringManager();
	scManager->SetVerboseLevel(VERBOSE_SCORING);

	// Set mandatory initialization classes
	// Detector construction
	B1DetectorConstruction* theGeometry = 0;
	theGeometry = new B1RegularDetectorConstruction();
	runManager->SetUserInitialization(theGeometry);

	// Physics list
	G4VModularPhysicsList* physicsList;
	if (PHANTOM_PRODUCTION_CUTS == 1){
		physicsList = new B1ModularPhysicsList("myList");
	}
	else{
		G4PhysListFactory factory;
		if (GT_MODE == 1){
			physicsList = factory.GetReferencePhysList("FTFP_BERT_LIV");
		} else{
			physicsList = factory.GetReferencePhysList("FTFP_BERT_EMV");
		}
			//FTFP_BERT_PEN
			//G4VModularPhysicsList* physicsList = new QBBC;
	}
	//physicsList->SetCutsForRegion(10*km,"waterRegion");

	//biasing
	G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();
	  if ( BIASING == 1 )
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

	physicsList->SetVerboseLevel(VERBOSE_PHYSICS_LIST);

	if (DETECTOR_SPECIAL_CUTS == 1) {
	physicsList->RegisterPhysics(new B1ExtraPhysics());
	}
	runManager->SetUserInitialization(physicsList);


	// User action initialization
	runManager->SetUserInitialization(new B1ActionInitialization(theGeometry));


		// Initialize visualization
	G4VisManager* visManager = new G4VisExecutive;
	// G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
	// G4VisManager* visManager = new G4VisExecutive("Quiet");
	visManager->SetVerboseLevel(VERBOSE_VIS);
	visManager->Initialize();

	// Get the pointer to the User Interface manager
	G4UImanager* UImanager = G4UImanager::GetUIpointer();
	 UImanager->ApplyCommand("/run/verbose " + IntToString(VERBOSE_RUN));
	 UImanager->ApplyCommand("/event/verbose " + IntToString(VERBOSE_EVENT));
	 UImanager->ApplyCommand("/tracking/verbose " + IntToString(VERBOSE_TRACK));


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
	if (PRINT_ELEMENTS_XS == 1){
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
	G4EmCalculator emCalculator;
	std::string elementName = el->GetName();
	G4int elementZ = el->GetZ();
	std::ofstream outputElementFile;

	std::string fileName =  "../run_outputs/ElementsXS/" + IntToString(elementZ) + ".csv";
	outputElementFile.open(fileName.c_str());

	G4double highE = PARTICLE_ENERGY*keV + 3*keV; //3 is buffer
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
