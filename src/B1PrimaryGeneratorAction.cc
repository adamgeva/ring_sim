
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
#include <iostream>
#include <cmath>

#include <string>
#include <sstream>
#include <fstream>


B1PrimaryGeneratorAction::B1PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fEnvelopeBox(0)
{
  params parameters;
  G4int n_particle = 1;
  // default particle kinematic
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName="gamma");

  // sourceAngleDiff is the angle between every source
  G4double sourceAngleDiff = 2*M_PI/NUM_OF_SOURCES;

  //writing sources locations to file
  std::ofstream outputSources;
  std::string fileName = "sourcesPos.csv";
  outputSources.open(fileName.c_str());
  outputSources << "Sources Location" << "\n";

  //place guns
  for (G4int i=0; i<NUM_OF_SOURCES; i++){
	  fParticleGun[i]  = new G4ParticleGun(n_particle);
	  fParticleGun[i]->SetParticleDefinition(particle);
	  fParticleGun[i]->SetParticleEnergy(parameters.MyparamsGun.particleEnergy);
	  //setting positions for all sources
	  G4double alpha = sourceAngleDiff*i;
	  G4double x0 = parameters.MyparamsGeometry.radius*(std::cos(alpha+M_PI));
	  G4double y0 = parameters.MyparamsGeometry.radius*(std::sin(alpha+M_PI));
	  G4double z0 = 0;
	  fParticleGun[i]->SetParticlePosition(G4ThreeVector(x0,y0,z0));
	  //write location to file
	  outputSources << x0/m << "," << y0/m << "," << z0/m << "\n";
  }

  outputSources.close();
}


B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
	for (G4int i=0; i<NUM_OF_SOURCES; i++){
		delete fParticleGun[i];
	}

}


void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	params parameters;
	G4int runID = G4RunManager::GetRunManager()->GetNonConstCurrentRun()->GetRunID();

	G4double MinTheta = 0 ;
	G4double MaxTheta = M_PI/6;
//	G4double MinTheta = M_PI/2  ;
//	G4double MaxTheta = M_PI/2 ;
//	G4double MinPhi = -M_PI/8;
//	G4double MaxPhi = M_PI/8;
	G4double MinPhi = 0;
	G4double MaxPhi = 2*M_PI;


	//phi
	G4double rndm1 = G4UniformRand();
	G4double phi = MinPhi + rndm1 * (MaxPhi - MinPhi);
	//cos,sin theta
	G4double rndm = G4UniformRand();
	G4double costheta = std::cos(MinTheta) - rndm * (std::cos(MinTheta) - std::cos(MaxTheta));
	G4double sintheta = std::sqrt(1. - costheta*costheta);
	//cos,sin phi
	G4double cosphi = std::cos(phi);
	G4double sinphi = std::sin(phi);
	//coordinates
	G4double px = costheta;
	G4double py = sintheta * cosphi;
	G4double pz = sintheta * sinphi;


	//if switching sources is applied
	if (ALT_SOURCES==1){
		// sourceAngleDiff is the angle between every source
		G4double sourceAngleDiff = 2*M_PI/NUM_OF_SOURCES;
		G4double angleTransform = sourceAngleDiff*runID;
		//T is after transformation because of new source position
		G4double pxT = px*std::cos(angleTransform) - py*std::sin(angleTransform);
		G4double pyT = px*std::sin(angleTransform) + py*std::cos(angleTransform);
		fParticleGun[runID]->SetParticleMomentumDirection(G4ThreeVector(pxT,pyT,pz));
		fParticleGun[runID]->GeneratePrimaryVertex(anEvent);
	}
	else {
		fParticleGun[0]->SetParticleMomentumDirection(G4ThreeVector(px,py,pz));
		fParticleGun[0]->GeneratePrimaryVertex(anEvent);
	}
}

