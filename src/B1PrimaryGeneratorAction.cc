
#include "params.hh"
#include "B1PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <iostream>
#include <cmath>


B1PrimaryGeneratorAction::B1PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fParticleGun(0), 
  fEnvelopeBox(0)
{
  params parameters;
  G4int n_particle = 1;
  fParticleGun  = new G4ParticleGun(n_particle);

  // default particle kinematic
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName="gamma");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleEnergy(parameters.MyparamsGun.particleEnergy);
}


B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
  delete fParticleGun;
}


void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	params parameters;

	G4double x0 = -parameters.MyparamsGeometry.radius;
	//G4double x0 = 0;
	G4double y0 = 0;
	G4double z0 = 0;


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
	G4double py = sintheta * sinphi;
	G4double pz = sintheta * cosphi;




	fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px,py,pz));
	//fParticleGun->SetParticleMomentumDirection(G4ThreeVector(1,0,0));
	fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));
	fParticleGun->GeneratePrimaryVertex(anEvent);
}

