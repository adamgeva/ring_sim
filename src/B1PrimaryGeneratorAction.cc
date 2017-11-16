
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
  //place gun
  fParticleGun  = new G4ParticleGun(n_particle);
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleEnergy(parameters.MyparamsGun.particleEnergy);
  //setting positions for all sources
  G4double alpha = 0;
  G4double x0 = parameters.MyparamsGeometry.radius*(std::cos(alpha+M_PI));
  G4double y0 = 0;
  G4double z0 = 0;
  fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));


}


B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
	delete fParticleGun;


}


void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{

	//coordinates
	G4double px = 1;
	G4double py = 0;
	G4double pz = 0;

	fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px,py,pz));
	fParticleGun->GeneratePrimaryVertex(anEvent);

}

