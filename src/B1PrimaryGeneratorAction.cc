
#include "params.hh"
#include "B1PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4ios.hh"
#include "globalFunctions.hh"
#include <iostream>
#include <cmath>
#include <time.h>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>



B1PrimaryGeneratorAction::B1PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fEnvelopeBox(0)
{
	readSpectrum();
	//initialize fenergyFlag
//	for (int l =0; l < NUM_OF_SPECTRUM_BINS; l ++){
//		fspect_test[l] = 0;
//	}

//	std::string fileName_spect = std::string(OUTPUT_DIR) + "/spectrum_output.csv";
//	foutput_spect.open(fileName_spect.c_str());

	//writing to file the chosen source
	/*
	fAllSources = 0;

	srand (time(NULL));
	frunIDRand = rand() % NUM_OF_SOURCES;
	//writing source chosen to file
	std::ofstream outputChosenSource;
	std::string fileNameSourceChosen = "../run_outputs_geom/sourceChosen.csv";
	outputChosenSource.open(fileNameSourceChosen.c_str());
	outputChosenSource << IntToString(frunIDRand) << "\n";
	outputChosenSource.close();

	G4String fname = FILE_FIXED_SOURCE;
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	if( !fin.is_open() ) {
	   G4Exception("Can't read change_sources_file",
					"",
					FatalErrorInArgument,
					G4String("File not found " + fname ).c_str());
	  }
	fin >> fAllSources;

*/
  //calculations for file export

  G4int n_particle = 1;
  // default particle kinematic
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName="gamma");
  //TODO: need to fix prints
  //writing sources locations to file
  /*
  std::ofstream outputSources;
  std::string fileName = FILE_SOURCE_POS;
  outputSources.open(fileName.c_str());
  outputSources << "Sources Location" << "\n";

  //writing source to detectors to file
  std::ofstream sourceToDet;
  std::string fileName_sToD = FILE_SOURCE_TO_DET;
  sourceToDet.open(fileName_sToD.c_str());
  sourceToDet << "Active detectors for every source" << "\n";
*/
  //place gun
  fParticleGun  = new G4ParticleGun(n_particle);
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleEnergy(PARTICLE_ENERGY*keV);
  //setting positions for all sources
  G4double x0 = -SOURCE_TO_CENTER*mm;
  G4double y0 = 0;
  G4double z0 = 0;
  fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));
/*
  //write location to file
  outputSources << x0/m << "," << y0/m << "," << z0/m << "\n";

	  
  outputSources.close();
  sourceToDet.close();
  */
}




B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
	delete fParticleGun;

}

void B1PrimaryGeneratorAction::readSpectrum(){
	G4String fname = FILE_SPECTRUM;
	G4double spectrum_dist[NUM_OF_SPECTRUM_BINS]; //distribution of energy for every source
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	G4double num_of_photons = 0;
	if( !fin.is_open() ) {
	   G4Exception("Can't read spectrum_file",
					"",
					FatalErrorInArgument,
					G4String("File not found " + fname ).c_str());
	}
    // Setup the weights (in this case linearly weighted)
	for (int i = 0; i<NUM_OF_SPECTRUM_BINS; i ++){
		if( fin.eof() ) break;
		fin >> num_of_photons;
		//std::cout << "i= " << i << " spect = " <<  num_of_photons << std::endl;
		spectrum_dist[i] = ceil(num_of_photons*NUM_OF_PHOTONS);
        fweights.push_back(spectrum_dist[i]);
	}
//	G4int sum = 0;
//	for (int k = 0; k<NUM_OF_SPECTRUM_BINS; k ++){
//		std::cout << "i= " << k << " spect = " <<  fspectrum_dist[k][0] << std::endl;
//		sum = sum + fspectrum_dist[k][0];
//	}
}

void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	G4double MinTheta = MIN_THETA;
	G4double MaxTheta = MAX_THETA;
//	G4double MinTheta = M_PI/2  ;
//	G4double MaxTheta = M_PI/2 ;
//	G4double MinPhi = -M_PI/8;
//	G4double MaxPhi = M_PI/8;
	G4double MinPhi = MIN_PHI;
	G4double MaxPhi = MAX_PHI;


	//phi
	G4double rndm1 = G4UniformRand();
	G4double phi = MinPhi + rndm1 * (MaxPhi - MinPhi);
	//cos,sin theta - used for cone beam
	G4double rndm = G4UniformRand();
	G4double costheta = std::cos(MinTheta) - rndm * (std::cos(MinTheta) - std::cos(MaxTheta));
	G4double sintheta = std::sqrt(1. - costheta*costheta);
	//cos,sin theta - used for fan beam
	//G4double rndm = G4UniformRand();
	//G4double theta = -MaxTheta + rndm*(2*MaxTheta);
	//G4double costheta = std::cos(theta);
	//G4double sintheta = std::sin(theta);
	//cos,sin phi
	G4double cosphi = std::cos(phi);
	G4double sinphi = std::sin(phi);
	//coordinates
	G4double px = costheta;
	G4double py = sintheta * cosphi;
	G4double pz = sintheta * sinphi;

	G4double energyP;
	G4int energyBin;

	fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px,py,pz));
	energyBin = getEnergyInd();
	energyP = energyBin*keV;
	fParticleGun->SetParticleEnergy(energyP);
	//fParticleGun->SetParticleEnergy(60*keV);

	fParticleGun->GeneratePrimaryVertex(anEvent);
	

	//
//	fspect_test[energyBin] = fspect_test[energyBin] + 1; // testing
}

G4int B1PrimaryGeneratorAction::getEnergyInd(){
	//todo - maybe we can declare discrete_distribution globally to avoid initialization
	std::discrete_distribution<G4int> distribution(fweights.begin(), fweights.end());
	G4int bin = distribution(fgenerator);
	return bin;
}


