#include "params.hh"
#include "B1DetectorConstruction.hh"
#include "myDetectorSD.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4VSensitiveDetector.hh"

#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"

#include "G4SDManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4ios.hh"
#include "globalFunctions.hh"
#include "B1EnergyDeposit.hh"
#include <math.h>


B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  detectorPixelLV(0),fVisAttributes()
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
	//todo: make generic and not hard coded
	G4Material* waterMat = nist->FindOrBuildMaterial("G4_WATER");
	G4Tubs* waterPhantomS = new G4Tubs("water_phantom",0,10*cm,50*cm,0,2*M_PI);
	G4LogicalVolume* waterPhantomLV = new G4LogicalVolume(waterPhantomS,waterMat,"water_phantom");
	G4RotationMatrix* rot = new G4RotationMatrix();
	rot->rotateX(-M_PI/2);
	new G4PVPlacement(rot,G4ThreeVector(),waterPhantomLV,"water_phantom",worldLV,false,0,checkOverlaps);

	//bone
	//todo: make generic and not hard coded
	G4Material* boneMat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
	G4ThreeVector posBone = G4ThreeVector(0, 0, 0);
	G4Tubs* boneS = new G4Tubs("bone",0, 2*cm, 50*cm,0,2*M_PI);
	G4LogicalVolume* boneLV = new G4LogicalVolume(boneS,boneMat,"bone");
	new G4PVPlacement(0,posBone,boneLV,"bone",waterPhantomLV,false,0,checkOverlaps);

	// detector - specs
	G4Material* detectorMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
	G4double detector_sizeX = parameters.MyparamsGeometry.detectorX;
	G4double detector_sizeY = parameters.MyparamsGeometry.detectorY;
	G4double detector_sizeZ = parameters.MyparamsGeometry.detectorZ;
	G4double radius = parameters.MyparamsGeometry.radius + parameters.MyparamsGeometry.detectorZ;

	// detector1
	G4Box* detectorS = new G4Box("detector",detector_sizeX, detector_sizeY, detector_sizeZ);
	G4LogicalVolume* detectorLV = new G4LogicalVolume(detectorS, detectorMat,"detector");

	//calculating angle between every detector
	G4double alpha = 2*atan(parameters.MyparamsGeometry.detectorX/parameters.MyparamsGeometry.radius);

	G4int numOfItr = (2*M_PI)/alpha;
	//correct for numeric errors - gap is spread
	alpha = (2*M_PI)/numOfItr;
	//initial location
	//G4int X = parameters.MyparamsGeometry.detectorY + parameters.MyparamsGeometry.shift;
	for (G4int i=0;i<numOfItr;i++)
	{
		G4double theta = i*alpha;
		//G4ThreeVector detectorPosUpdated = G4ThreeVector(cos(theta)*(radius), -X +j*2*X, sin(theta)*(radius));
		G4ThreeVector detectorPosUpdated = G4ThreeVector(cos(theta)*(radius), 0, sin(theta)*(radius));
		G4RotationMatrix* rotD = new G4RotationMatrix();
		rotD->rotateY(-M_PI/2+theta);
		new G4PVPlacement(rotD,detectorPosUpdated,detectorLV,"detector",worldLV,false,i,checkOverlaps);
	}

	//Pixel
	G4Box* detectorPixelS = new G4Box("detectorCell",detector_sizeX, detector_sizeY/parameters.MyparamsGeometry.numberOfRows, detector_sizeZ);
	detectorPixelLV = new G4LogicalVolume(detectorPixelS, detectorMat,"detectorPixel");
	new G4PVReplica("detectorPixelP",detectorPixelLV,detectorLV,kYAxis,parameters.MyparamsGeometry.numberOfRows,2*detector_sizeY/parameters.MyparamsGeometry.numberOfRows);

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
	params parameters;

	// sensitive detectors
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	G4String SDname;

	//detector1 - SD
	//detector2 - Scorer
	//creating my sensitive detector and adding it to the SD manager
	G4VSensitiveDetector* detector1 = new myDetectorSD(SDname="/detector1");
	SDman->AddNewDetector(detector1);
	//attaching my sensitive detector to the detector logical element
	SetSensitiveDetector(detectorPixelLV,detector1);
	//creating scorer
	G4MultiFunctionalDetector* detector2 = new G4MultiFunctionalDetector("detector2");
	SDman->AddNewDetector(detector2);
	// setting primitive scorers
	for (G4int i=0; i<NUM_OF_SCORERS; i++){
		G4VPrimitiveScorer* primitive;
		primitive = new B1EnergyDeposit("eDep_" + IntToString(i),i);
	    detector2->RegisterPrimitive(primitive);
	}
    SetSensitiveDetector(detectorPixelLV,detector2);
}
