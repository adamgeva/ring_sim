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

#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "B1BOptrMultiParticleChangeCrossSection.hh"
#include "B1BOptrFS.hh"


#include "G4BOptrForceCollision.hh"


#include "G4UserLimits.hh"

#include <math.h>
#include <iostream>



B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),

  fvoxel_logic(0),
  fContainer_solid(0),
  fContainer_logic(0),
  fContainer_phys(0),
  fMateIDs(0),
  fNVoxelX(0),
  fNVoxelY(0),
  fNVoxelZ(0),
  fVoxelHalfDimX(0),
  fVoxelHalfDimY(0),
  fVoxelHalfDimZ(0),
  worldLV(),
  detectorPixelLV(0),
  fVisAttributes()
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

	//set tolerance
	G4GeometryManager::GetInstance()->SetWorldMaximumExtent(2.*parameters.MyparamsGeometry.worldXY);
	std::cout << "GetSurfaceTolerance() = " << G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()/mm << std::endl;
	std::cout << "GetAngularTolerance() = " << G4GeometryTolerance::GetInstance()->GetAngularTolerance()/mm << std::endl;
	std::cout << "GetRadialTolerance() = " << G4GeometryTolerance::GetInstance()->GetRadialTolerance()/mm << std::endl;


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
	worldLV = new G4LogicalVolume(worldS, world_mat, "World");
	G4VPhysicalVolume* worldPHS = new G4PVPlacement(0, G4ThreeVector(),worldLV,"World",0,false,0,checkOverlaps);

	// detector - specs
	G4Material* detectorMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
	G4double detector_sizeX = parameters.MyparamsGeometry.detectorX;
	G4double detector_sizeY = parameters.MyparamsGeometry.detectorY;
	G4double detector_sizeZ = parameters.MyparamsGeometry.detectorZ;
	G4double radius = parameters.MyparamsGeometry.radius + parameters.MyparamsGeometry.detectorZ;

	//vars used for pos calculations
	G4double r = parameters.MyparamsGeometry.radius;
	G4double beta = atan(detector_sizeX/r);
	G4double beta_tag = atan(detector_sizeX/(2*r));
	G4double R = sqrt(pow(r,2)+pow(detector_sizeX,2));
	G4double R_tag = sqrt(pow(r,2)+pow(detector_sizeX/2,2));

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
	G4cout << "numOfItr" << numOfItr <<G4endl;

	//writing detector locations to file
	std::ofstream outputDet;
	std::string fileName = "../run_outputs/detectorsPos.csv";
	outputDet.open(fileName.c_str());
	outputDet << "Detectors Location" << "\n";

	for (G4int i=0;i<numOfItr;i++)
	{
		G4double theta = i*alpha;
		//G4ThreeVector detectorPosUpdated = G4ThreeVector(cos(theta)*(radius), -X +j*2*X, sin(theta)*(radius));
		G4ThreeVector detectorPosUpdated = G4ThreeVector(cos(theta)*(radius),sin(theta)*(radius),0);
		G4RotationMatrix* rotD = new G4RotationMatrix();
		rotD->rotateX(-M_PI/2);
		//rotD->rotateY(-M_PI/2+theta);
		rotD->rotateY(M_PI/2-theta);
		if (parameters.MyparamsGeometry.buildDetectors == 1){
			new G4PVPlacement(rotD,detectorPosUpdated,detectorLV,"detector",worldLV,false,i,checkOverlaps);
		}
		//writing 5 pos for every detector - one detector in every line of the file
		G4double x1 = R*cos(theta-beta);
		G4double y1 = R*sin(theta-beta);
		G4double x2 = R_tag*cos(theta-beta_tag);
		G4double y2 = R_tag*sin(theta-beta_tag);
		G4double x3 = r*cos(theta);
		G4double y3 = r*sin(theta);
		G4double x4 = R_tag*cos(theta+beta_tag);
		G4double y4 = R_tag*sin(theta+beta_tag);
		G4double x5 = R*cos(theta+beta);
		G4double y5 = R*sin(theta+beta);
		outputDet<<x1/m<<","<<y1/m<<","<<x2/m<<","<<y2/m<<","<<x3/m<<","<<y3/m<<","<<x4/m<<","<<y4/m<<","<<x5/m<<","<<y5/m<<"\n";
	}

	outputDet.close();

	//Pixel
	G4Box* detectorPixelS = new G4Box("detectorCell",detector_sizeX, detector_sizeY/parameters.MyparamsGeometry.numberOfRows, detector_sizeZ);
	detectorPixelLV = new G4LogicalVolume(detectorPixelS, detectorMat,"detectorPixel");
	new G4PVReplica("detectorPixelP",detectorPixelLV,detectorLV,kYAxis,parameters.MyparamsGeometry.numberOfRows,2*detector_sizeY/parameters.MyparamsGeometry.numberOfRows);



	//world
	//setting visualization attributes to logical elements
	G4VisAttributes* visAttributes = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
	visAttributes->SetVisibility(false);
	worldLV->SetVisAttributes(visAttributes);
	fVisAttributes.push_back(visAttributes);

	//detector
	visAttributes = new G4VisAttributes(G4Colour(0.9,0.9,0.9));
	detectorLV->SetVisAttributes(visAttributes);
	fVisAttributes.push_back(visAttributes);

	//building phantom
	if (parameters.MyparamsGeometry.buildPhantom==1){
		//initialize materials
		ReadPhantomDataAndInitialisationOfMaterials();
		ConstructPhantomContainer();
		ConstructPhantom();
	}

	// User Limits

  // Set additional contraints on the track, with G4UserSpecialCuts
  //
   G4double maxStep=DBL_MAX, maxLength = DBL_MAX, maxTime = DBL_MAX, minEkin = parameters.MyparamsGun.particleEnergy + 5*keV;
   detectorPixelLV->SetUserLimits(new G4UserLimits(maxStep,maxLength,maxTime,minEkin));
//   if (parameters.MyparamsGeometry.buildPhantom==1){
//	   fvoxel_logic->SetUserLimits(new G4UserLimits(maxStep,maxLength,maxTime,minEkin));
//   }

 //-------------------regions-------------------

//   G4Region* Container_region = new G4Region("ContainerRegion");
//   fContainer_logic->SetRegion(Container_region);
//   Container_region->AddRootLogicalVolume(fContainer_logic);



	//always return the physical World
	return worldPHS;
}


void B1DetectorConstruction::ConstructPhantomContainer()
{
	params parameters;

  //---- Extract number of voxels and voxel dimensions
  fNVoxelX = parameters.MyparamsGeometry.numberOfVoxelsX;
  fNVoxelY = parameters.MyparamsGeometry.numberOfVoxelsY;
  fNVoxelZ = parameters.MyparamsGeometry.numberOfZSlices;

  fVoxelHalfDimX = parameters.MyparamsGeometry.voxelHalfX;
  fVoxelHalfDimY = parameters.MyparamsGeometry.voxelHalfY;
  fVoxelHalfDimZ = parameters.MyparamsGeometry.voxelHalfZ;


  //----- Define the volume that contains all the voxels
  fContainer_solid = new G4Box("phantomContainer",fNVoxelX*fVoxelHalfDimX,
                               fNVoxelY*fVoxelHalfDimY,
                               fNVoxelZ*fVoxelHalfDimZ);
  fContainer_logic =
    new G4LogicalVolume( fContainer_solid,
   //the material is not important, it will be fully filled by the voxels
                         fMaterials[0],
                         "phantomContainer",
                         0, 0, 0 );
  //--- Place it on the world

  fContainer_phys =
    new G4PVPlacement(0,  // rotation
    				  G4ThreeVector(),
                      fContainer_logic,     // The logic volume
                      "phantomContainer",  // Name
                      worldLV,  // Mother
                      false,           // No op. bool.
                      1);              // Copy number

  //fContainer_logic->SetVisAttributes(new G4VisAttributes(G4Colour(1.,0.,0.)));
}


void B1DetectorConstruction::ReadPhantomDataAndInitialisationOfMaterials()
{
    // Creating elements :
    G4double z, a, density;
    G4String name, symbol;
    G4int numberofElements;
    params parameters;

    // initiallize fMateIDs
    fMateIDs = new size_t[NUM_OF_VOXELS];


    G4Element* elH = new G4Element( name = "Hydrogen",
                                   symbol = "H",
                                   z = 1.0, a = 1.008  * g/mole );
    G4Element* elHe = new G4Element( name = "Helium",
                                   symbol = "He",
                                   z = 2.0, a = 4.0026  * g/mole );
    G4Element* elLi = new G4Element( name = "Lithium",
                                   symbol = "Li",
                                   z = 3.0, a = 6.941  * g/mole );
    G4Element* elBe = new G4Element( name = "Beryllium",
                                   symbol = "Be",
                                   z = 4.0, a = 9.012182  * g/mole );
    G4Element* elB = new G4Element( name = "Boron",
								   symbol = "B",
								   z = 5.0, a = 10.811  * g/mole );
    G4Element* elC = new G4Element( name = "Carbon",
								   symbol = "C",
								   z = 6.0, a = 12.011 * g/mole );
    G4Element* elN = new G4Element( name = "Nitrogen",
                                   symbol = "N",
                                   z = 7.0, a = 14.007 * g/mole );
    G4Element* elO = new G4Element( name = "Oxygen",
                                   symbol = "O",
                                   z = 8.0, a = 16.00  * g/mole );
    G4Element* elF = new G4Element( name = "Fluorine",
								   symbol = "F",
								   z = 9.0, a = 18.998404  * g/mole );
    G4Element* elNe = new G4Element( name = "Neon",
								   symbol = "Ne",
								   z = 10.0, a = 20.1797  * g/mole );
    G4Element* elNa = new G4Element( name = "Sodium",
                                    symbol = "Na",
                                    z= 11.0, a = 22.98977 * g/mole );
    G4Element* elMg = new G4Element( name = "Magnesium",
									symbol = "Mg",
									z= 12.0, a = 24.305 * g/mole );
    G4Element* elAl = new G4Element( name = "Aluminum",
									symbol = "Al",
									z= 13.0, a = 26.981539 * g/mole );
    G4Element* elP = new G4Element( name = "Phosphorus",
									symbol = "P",
									z= 15.0, a = 30.97376 * g/mole );
    G4Element* elS = new G4Element( name = "Sulfur",
                                   	symbol = "S",
                                   	z = 16.0,a = 32.065* g/mole );
    G4Element* elCl = new G4Element( name = "Chlorine",
                                    symbol = "Cl",
                                    z = 17.0, a = 35.453* g/mole );
    G4Element* elAr = new G4Element( name = "Argon",
									symbol = "Ar",
									z= 18.0, a = 39.948 * g/mole );
    G4Element* elK = new G4Element( name = "Potassium",
                                   	symbol = "K",
                                   	z = 19.0, a = 39.0983* g/mole );
    G4Element* elCa = new G4Element( name="Calcium",
                                    symbol = "Ca",
                                    z = 20.0, a = 40.078* g/mole );
    G4Element* elSc = new G4Element( name="Scandium",
                                    symbol = "Sc",
                                    z = 21.0, a = 44.95591 * g/mole );
    G4Element* elTi = new G4Element( name="Titanium",
                                    symbol = "Ti",
                                    z = 22.0, a = 47.867 * g/mole );
    G4Element* elV = new G4Element( name="Vanadium",
                                    symbol = "V",
                                    z = 23.0, a = 50.9415 * g/mole );
    G4Element* elCr = new G4Element( name="Chromium",
                                    symbol = "Cr",
                                    z = 24.0, a = 51.9961 * g/mole );
    G4Element* elMn = new G4Element( name="Manganese",
                                    symbol = "Mn",
                                    z = 25.0, a = 54.93805 * g/mole );
    G4Element* elFe = new G4Element( name = "Iron",
                                    symbol = "Fe",
                                    z = 26, a = 55.845* g/mole );
    G4Element* elI = new G4Element( name = "Iodine",
                                    symbol = "I",
                                    z = 53, a = 126.90447 * g/mole );
    G4Element* elPb = new G4Element( name = "Lead",
                                    symbol = "Pb",
                                    z = 82, a = 207.2 * g/mole );

//*******************************************************************************************************************************************************
	G4String fname = parameters.MyparamsGeometry.voxels_materials_file;
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	if( !fin.is_open() ) {
	   G4Exception("Can't read voxels_materials_file",
					"",
					FatalErrorInArgument,
					G4String("File not found " + fname ).c_str());
	  }
	//pointers to materials - number of voxels
	G4Material* material;
	G4String MaterialName[NUM_OF_VOXELS];
	//iterate over the voxels - every voxel as a material
	G4int voxel = 0;
	for (voxel = 0; voxel<parameters.MyparamsGeometry.numberOfPixelsPerSlice*parameters.MyparamsGeometry.numberOfZSlices; voxel ++){
		if( fin.eof() ) break;
		// material - place in an array
		MaterialName[voxel] = "mat" + IntToString(voxel);
		G4double rau;
		G4double fracs[NUM_OF_ELEMENTS];
		// read density
		fin >> rau;
		// create material
//		material[voxel] = new G4Material( name = MaterialName[voxel],
//											   density = rau*g/cm3,
//											   numberofElements = parameters.Myparams.numberOfElements );
		material = new G4Material( name = MaterialName[voxel],
											   density = rau*g/cm3,
											   numberofElements = 2);
		std::cout << "voxel number: " << voxel << " rho: " << rau << " fractions: ";
		// read fractions
		for (G4int i=0; i<parameters.Myparams.numberOfElements; i++){
			fin >> fracs[i];
			std::cout << fracs[i] << ",";
		}
		std::cout << "." << std::endl;

		//adding elements according to fractions
//		material[voxel]->AddElement(elH,fracs[0]);
//		material[voxel]->AddElement(elHe,fracs[1]);
//		material[voxel]->AddElement(elLi,fracs[2]);
//		material[voxel]->AddElement(elBe,fracs[3]);
//		material[voxel]->AddElement(elB,fracs[4]);
//		material[voxel]->AddElement(elC,fracs[5]);
//		material[voxel]->AddElement(elN,fracs[6]);
		material->AddElement(elO,fracs[7]);
//		material[voxel]->AddElement(elF,fracs[8]);
//		material[voxel]->AddElement(elNe,fracs[9]);
//		material[voxel]->AddElement(elNa,fracs[10]);
//		material[voxel]->AddElement(elMg,fracs[11]);
//		material[voxel]->AddElement(elAl,fracs[12]);
//		material[voxel]->AddElement(elP,fracs[13]);
//		material[voxel]->AddElement(elS,fracs[14]);
//		material[voxel]->AddElement(elCl,fracs[15]);
//		material[voxel]->AddElement(elAr,fracs[16]);
//		material[voxel]->AddElement(elK,fracs[17]);
		material->AddElement(elCa,fracs[18]);
//		material[voxel]->AddElement(elSc,fracs[19]);
//		material[voxel]->AddElement(elTi,fracs[20]);
//		material[voxel]->AddElement(elV,fracs[21]);
//		material[voxel]->AddElement(elCr,fracs[22]);
//		material[voxel]->AddElement(elMn,fracs[23]);
//		material[voxel]->AddElement(elFe,fracs[24]);
//		material[voxel]->AddElement(elI,fracs[25]);
//		material[voxel]->AddElement(elPb,fracs[26]);

		//add material to fMaterials
		fMaterials.push_back(material);
		//connecting the voxel to material
		fMateIDs[voxel] = voxel;
	}
}

void B1DetectorConstruction::ConstructSDandField()
{
	params parameters;

  // -- Fetch volume for biasing:
 // G4LogicalVolume* logicTest = G4LogicalVolumeStore::GetInstance()->GetVolume("phantomContainer");
  //G4LogicalVolume* logicWorld = G4LogicalVolumeStore::GetInstance()->GetVolume("World");
  //G4LogicalVolume* logicTestBone = G4LogicalVolumeStore::GetInstance()->GetVolume("bone");

//  // ----------------------------------------------
//  // -- operator creation and attachment to volume:
//  // ----------------------------------------------
//  B1BOptrMultiParticleChangeCrossSection* testMany = new B1BOptrMultiParticleChangeCrossSection();
//  testMany->AddParticle("gamma");
//  testMany->AttachTo(logicTest);
//  testMany->AttachTo(logicWorld);
//   ----------------------------------------------
//   -- operator creation and attachment to volume:
//   ----------------------------------------------
//  B1BOptrFS* FSOptr =  new B1BOptrFS("gamma","FSOperator");
//  FSOptr->AttachTo(logicTest);
//  //FSOptr->AttachTo(logicWorld);
//  //comptLEOptr->AttachTo(logicTestBone);
//  G4cout << " Attaching biasing operator " << FSOptr->GetName()
//         << " to logical volume " << logicTest->GetName()
//         << G4endl;

//  G4BOptrForceCollision* OptrForceCollision =  new G4BOptrForceCollision("gamma","forceCollision");
//  OptrForceCollision->AttachTo(logicTest);
//  //OptrForceCollision->AttachTo(logicTestBone);


	// sensitive detectors
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	//detector1 - SD
	//detector2 - Scorer
	//creating my sensitive detector and adding it to the SD manager - the data will be saved in histograms only if record hist is on
	if (parameters.Myparams.recordHist==1){
		G4String SDname;
		G4VSensitiveDetector* detector1 = new myDetectorSD(SDname="/detector1");
		SDman->AddNewDetector(detector1);
		//attaching my sensitive detector to the detector logical element
		SetSensitiveDetector(detectorPixelLV,detector1);
	}
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





