#include "params.hh"
#include "B1DetectorConstruction.hh"
#include "myDetectorSD.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include <math.h>


B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  detectorLV(0),fVisAttributes()
{ }

B1DetectorConstruction::~B1DetectorConstruction()
{
	for (G4int i=0; i<G4int(fVisAttributes.size()); ++i)
	    {
	      delete fVisAttributes[i];
	    }
}


G4VPhysicalVolume* B1DetectorConstruction::Construct()
{  
	params parameters;
	// Get nist material manager
	G4NistManager* nist = G4NistManager::Instance();
	// Option to switch on/off checking of volumes overlaps
	G4bool checkOverlaps = true;

	// World
	// sizes are half size
	G4double world_sizeXY = parameters.MyparamsGeometry.worldXY;
	G4double world_sizeZ  = parameters.MyparamsGeometry.worldZ;
	G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

	G4Box* worldS = new G4Box("World",world_sizeXY, world_sizeXY, world_sizeZ);
	G4LogicalVolume* worldLV = new G4LogicalVolume(worldS, world_mat, "World");
	G4VPhysicalVolume* worldPHS = new G4PVPlacement(0, G4ThreeVector(),worldLV,"World",0,false,0,checkOverlaps);


	// Water phantom

	G4Material* waterMat = nist->FindOrBuildMaterial("G4_WATER");
	G4Box* waterPhantomS = new G4Box("water_phantom",parameters.MyparamsGeometry.phantomXY,parameters.MyparamsGeometry.phantomXY,parameters.MyparamsGeometry.phantomZ);
	G4LogicalVolume* waterPhantomLV = new G4LogicalVolume(waterPhantomS,waterMat,"water_phantom");
	new G4PVPlacement(0,G4ThreeVector(),waterPhantomLV,"water_phantom",worldLV,false,0,checkOverlaps);

	//bone
	G4Material* boneMat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
	G4ThreeVector posBone = G4ThreeVector(0, 0, 0);
	G4Box* boneS = new G4Box("bone",parameters.MyparamsGeometry.boneX, parameters.MyparamsGeometry.boneY, parameters.MyparamsGeometry.boneZ);
	G4LogicalVolume* boneLV = new G4LogicalVolume(boneS,boneMat,"bone");
	new G4PVPlacement(0,posBone,boneLV,"bone",waterPhantomLV,false,0,checkOverlaps);

	// detector - specs
	G4Material* detectorMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
	G4double detector_sizeXY = parameters.MyparamsGeometry.detectorXY;
	G4double detector_sizeZ = parameters.MyparamsGeometry.detectorZ;
	G4double radius = parameters.MyparamsGeometry.radius;

	//Todo: create detectors in a loop - generic way!
	// detector1
	G4Box* detectorS = new G4Box("detector",detector_sizeXY, detector_sizeXY, detector_sizeZ);
	detectorLV = new G4LogicalVolume(detectorS, detectorMat,"detector");
	//initial location
	for (G4int i=0;i<parameters.MyparamsGeometry.numberOfDetectors;i++)
	{
		G4double theta = i*(M_PI/4);
		G4ThreeVector detectorPosUpdated = G4ThreeVector(cos(theta)*(radius), 0, sin(theta)*(radius));
		//G4ThreeVector detectorPosUpdated = G4ThreeVector(radius,0,0);
		G4RotationMatrix* rotD = new G4RotationMatrix();
		//rotD->rotateY(i*45.*deg);
		G4double angle = -90 + i*45;
		rotD->rotateY(angle*deg);
		new G4PVPlacement(rotD,detectorPosUpdated,detectorLV,"detector",worldLV,false,i,checkOverlaps);
	}


	/*
	//replicating colums
	detector1 - column
	G4Box* detectorColumnS = new G4Box("detectorColumnBox",detector_sizeXY/10,detector_sizeXY,detector_sizeZ);
	detectorColumnLV = new G4LogicalVolume(detectorColumnS,detectorMat,"detectorColumnLogical");
	new G4PVReplica("detectorColumnPhysical",detectorColumnLV,detectorLV,kXAxis,10,detector_sizeXY/5);
	 */

	//setting visualization attributes to logical elements
	//world
	G4VisAttributes* visAttributes = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
	visAttributes->SetVisibility(false);
	worldLV->SetVisAttributes(visAttributes);
	fVisAttributes.push_back(visAttributes);

	//detector
	visAttributes = new G4VisAttributes(G4Colour(0.9,0.9,0.9));
	detectorLV->SetVisAttributes(visAttributes);
	fVisAttributes.push_back(visAttributes);

	//bone
	visAttributes = new G4VisAttributes(G4Colour(0.8888,0.0,0.0));
	boneLV->SetVisAttributes(visAttributes);
	fVisAttributes.push_back(visAttributes);

	//water phantom
	visAttributes = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
	waterPhantomLV->SetVisAttributes(visAttributes);
	fVisAttributes.push_back(visAttributes);

	//always return the physical World
	return worldPHS;
}

void B1DetectorConstruction::ConstructSDandField()
{
	// sensitive detectors
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	G4String SDname;

	//creating my sensitive detector and adding it to the SD manager
	G4VSensitiveDetector* detector1 = new myDetectorSD(SDname="/detector");
	SDman->AddNewDetector(detector1);
	//attaching my sensitive detector to the detector logical element
	detectorLV -> SetSensitiveDetector(detector1);

}
