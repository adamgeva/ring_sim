
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
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,1.));
  fParticleGun->SetParticleEnergy(parameters.MyparamsGun.particleEnergy);
}


B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
  delete fParticleGun;
}


void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	params parameters;
	//this function is called at the begining of each event
	// In order to avoid dependence of PrimaryGeneratorAction
	// on DetectorConstruction class we get Envelope volume
	// from G4LogicalVolumeStore.

	//G4double envSizeXY = 0;
	//G4double envSizeZ = 0;

	G4double envSizeXY = parameters.MyparamsGeometry.detectorXY;
	G4double envSizeZ = parameters.MyparamsGeometry.phantomZ;

	//unmark to define source location wrt water phantom
	/*
	if (!fEnvelopeBox)
	{
	G4LogicalVolume* waterLV
	  = G4LogicalVolumeStore::GetInstance()->GetVolume("water_phantom");
	if ( waterLV ) fEnvelopeBox = dynamic_cast<G4Box*>(waterLV->GetSolid());
	}

	if ( fEnvelopeBox ) {
	envSizeXY = fEnvelopeBox->GetXHalfLength()*2.;
	envSizeZ = fEnvelopeBox->GetZHalfLength()*2.;
	}
	else  {
	G4ExceptionDescription msg;
	msg << "Envelope volume of box shape not found.\n";
	msg << "Perhaps you have changed geometry.\n";
	msg << "The gun will be place at the center.";
	G4Exception("B1PrimaryGeneratorAction::GeneratePrimaries()",
	 "MyCode0002",JustWarning,msg);
	}
	*/

	G4double size = parameters.MyparamsGun.detectorCoverage;
	G4double x0 = 2* size * envSizeXY * (G4UniformRand()-0.5);
	G4double y0 = 2* size * envSizeXY * (G4UniformRand()-0.5);
	G4double z0 = -envSizeZ;

	fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));
	fParticleGun->GeneratePrimaryVertex(anEvent);
}

