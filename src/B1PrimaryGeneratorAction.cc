
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
  //calculations for file export
  //calculating angle between two detectors - same calculation is done in detector construction
  G4double detectorAngleDiff = 2*atan(parameters.MyparamsGeometry.detectorX/parameters.MyparamsGeometry.radius);
  G4int numOfItr = (2*M_PI)/detectorAngleDiff;
  //correct for numeric errors - gap is spread
  detectorAngleDiff = (2*M_PI)/numOfItr;
  //beta is the angle coverage (cone) of the source
  G4double beta = 2*parameters.MyparamsGun.MaxTheta;
  // sourceAngleDiff is the angle between every source
  G4double sourceAngleDiff = 2*M_PI/NUM_OF_SOURCES;

  G4int n_particle = 1;
  // default particle kinematic
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName="gamma");



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

	//G4double MinTheta = parameters.MyparamsGun.MinTheta;
	G4double MaxTheta = parameters.MyparamsGun.MaxTheta;
//	G4double MinTheta = M_PI/2  ;
//	G4double MaxTheta = M_PI/2 ;
//	G4double MinPhi = -M_PI/8;
//	G4double MaxPhi = M_PI/8;
	G4double MinPhi = parameters.MyparamsGun.MinPhi;
	G4double MaxPhi = parameters.MyparamsGun.MaxPhi;


	//phi
	G4double rndm1 = G4UniformRand();
	G4double phi = MinPhi + rndm1 * (MaxPhi - MinPhi);
	//cos,sin theta - used for cone beam
//	G4double rndm = G4UniformRand();
//	G4double costheta = std::cos(MinTheta) - rndm * (std::cos(MinTheta) - std::cos(MaxTheta));
//	G4double sintheta = std::sqrt(1. - costheta*costheta);
	//cos,sin theta - used for fan beam
	G4double rndm = G4UniformRand();
	G4double theta = -MaxTheta + rndm*(2*MaxTheta);
	G4double costheta = std::cos(theta);
	G4double sintheta = std::sin(theta);
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

