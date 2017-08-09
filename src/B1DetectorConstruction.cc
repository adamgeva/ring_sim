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
#include <iostream>



B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  worldLV(),
  detectorPixelLV(0),
  fVisAttributes(),
  fNVoxelX(0),
  fNVoxelY(0),
  fNVoxelZ(0),
  fVoxelHalfDimX(0),
  fVoxelHalfDimY(0),
  fVoxelHalfDimZ(0),
  fContainer_solid(0),
  fContainer_logic(0),
  fContainer_phys(0),
  fNoFiles(0),
  fMateIDs(0)
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
	//initialize materials
    InitialisationOfMaterials();
    InitialisationOfMaterialsMap();

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
	std::string fileName = "detectorsPos.csv";
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
		new G4PVPlacement(rotD,detectorPosUpdated,detectorLV,"detector",worldLV,false,i,checkOverlaps);

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

	ReadPhantomData();
    ConstructPhantomContainer();
    ConstructPhantom();

	//always return the physical World
	return worldPHS;
}

void B1DetectorConstruction::ReadPhantomData()
{
	params parameters;

    fNoFiles = parameters.MyparamsGeometry.numberOfZSlices;
    G4int x=10000; //starting value for file names
    for(G4int i = 0; i < fNoFiles; i++ ) {
        //--- Read one data file
        G4String fileName = parameters.MyparamsGeometry.phantomFileName + IntToString(x+i) + ".txt";
        ReadPhantomDataFile(fileName,i);
    }
}

void B1DetectorConstruction::ReadPhantomDataFile(const G4String& fname, G4int sliceNumber)
{
	params parameters;

  std::cout << " B1DetectorConstruction::ReadPhantomDataFile opening file "
		 << fname << std::endl;
  //TODO: handle reading from phantom files
  std::ifstream fin(fname.c_str(), std::ios_base::in);
  if( !fin.is_open() ) {
    G4Exception("B1DetectorConstruction::ReadPhantomDataFile",
                "",
                FatalErrorInArgument,
                G4String("File not found " + fname ).c_str());
  }

  G4int nVoxels = parameters.MyparamsGeometry.numberOfPixelsPerSlice; //256*256

  //--- If first slice, initiliaze fMateIDs
  if( sliceNumber==0 ) {
    fMateIDs = new size_t[fNoFiles*nVoxels];
  }

  G4double mateID;
  // number of voxels from previously read slices
  G4int voxelCopyNo = (sliceNumber)*nVoxels;
  materialIDs mateStruct;
  G4double ID;
  for( G4int ii = 0; ii < nVoxels; ii++, voxelCopyNo++ ){
    fin >> ID;
    mateStruct = intensityToMateID[ID];
    mateID = mateStruct.mat1ID;
    if (mateID!=0) {
    	std::cout << "structureMateID = " << mateID << std::endl;
    	std::cout << "ii = " << ii << " voxelCopyNo = " << voxelCopyNo << std::endl;
    }
    fMateIDs[voxelCopyNo] = mateID;
  }



//    //-- Get material from list of original materials
//    mateID = fMateIDs[voxelCopyNo];
//    G4Material* mateOrig  = fMaterials[mateID];

}

void B1DetectorConstruction::ConstructPhantomContainer()
{
	params parameters;

  //---- Extract number of voxels and voxel dimensions
  fNVoxelX = parameters.MyparamsGeometry.numberOfVoxelsX;
  fNVoxelY = parameters.MyparamsGeometry.numberOfVoxelsY;
  fNVoxelZ = parameters.MyparamsGeometry.numberOfPixelsPerSlice;

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

void B1DetectorConstruction::InitialisationOfMaterialsMap()
{
	params parameters;
	G4String fname = parameters.MyparamsGeometry.IdToCompMapName;
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	if( !fin.is_open() ) {
	   G4Exception("B1DetectorConstruction::ReadPhantomDataFile",
	                "",
	                FatalErrorInArgument,
	                G4String("File not found " + fname ).c_str());
	  }
	//read header - dummy line
	std::string line;
	std::getline(fin,line);
	//std::cout << line << std::endl;
	//read file and create a map
	while (true) {
	    if( fin.eof() ) break;

	    G4int NumOfMaterials;
	    G4int ID;
	    G4int MateA;
	    G4int MateB;

	    fin >> NumOfMaterials;
	    fin >> ID;
	    //std::cout<<"NumOfMaterials: " << NumOfMaterials << " ID: " << ID << std::endl;
	    //create material struct
	    materialIDs MaterialStruct;
	    if (NumOfMaterials==1){
	    	fin >> MateA;
	    	//std::cout<<"MateA: " << MateA << std::endl;
	    	MaterialStruct.mat1ID=MateA;
	    	MaterialStruct.mat2ID=-1;
	    } else { //NumOfMaterials==2
	    	fin >> MateA;
	    	fin >> MateB;
	    	//std::cout<<"MateA: " << MateA << "MateB: " << MateB << std::endl;
	    	MaterialStruct.mat1ID=MateA;
	    	MaterialStruct.mat2ID=MateB;
	    }

	    //insert to map
	    std::pair<std::map<G4int,materialIDs>::iterator,bool> ret;
	    //std::cout << "inserting ID: " << ID << std::endl;
	    ret = intensityToMateID.insert ( std::pair<G4int,materialIDs>(ID,MaterialStruct) );
	    if (ret.second==false) {
	      std::cout << "element " << ID << " already existed" << std::endl;
	      //std::cout << " with a value of " << intensityToMateID[ret.first] << '\n';
	    }
	}
	//std::cout << "intensityToMateID.size() is " << intensityToMateID.size() << std::endl;

}

void B1DetectorConstruction::InitialisationOfMaterials()
{
    // Creating elements :
    G4double z, a, density;
    G4String name, symbol;


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


    // Creating Materials :
    G4int numberofElements;

    // Water
    G4Material* water = new G4Material( "Water",
                                       density = 1.0*g/cm3,
                                       numberofElements = 2 );
    water->AddElement(elH,0.112);
    water->AddElement(elO,0.888);

    // Muscle
	G4Material* muscle = new G4Material( "Muscle",
										density = 1.05*g/cm3,
										numberofElements = 9 );
	muscle->AddElement(elH,0.102);
	muscle->AddElement(elC,0.143);
	muscle->AddElement(elN,0.034);
	muscle->AddElement(elO,0.710);
	muscle->AddElement(elNa,0.001);
	muscle->AddElement(elP,0.002);
	muscle->AddElement(elS,0.003);
	muscle->AddElement(elCl,0.001);
	muscle->AddElement(elK,0.004);

    //  Lung Inhale
    G4Material* lung = new G4Material( "Lung",
                                            density = 0.3*g/cm3,
                                            numberofElements = 5);
    lung->AddElement(elH,0.099);
    lung->AddElement(elC,0.1);
    lung->AddElement(elN,0.028);
    lung->AddElement(elO,0.740);
    lung->AddElement(elP,0.001);

    // Dry spine
    G4Material* dry_spine = new G4Material( "DrySpine",
                                            density = 1.42*g/cm3,
                                            numberofElements = 11);
    dry_spine->AddElement(elH,0.063);
    dry_spine->AddElement(elC,0.261);
    dry_spine->AddElement(elN,0.039);
    dry_spine->AddElement(elO,0.436);
    dry_spine->AddElement(elNa,0.001);
    dry_spine->AddElement(elMg,0.001);
    dry_spine->AddElement(elP,0.061);
    dry_spine->AddElement(elS,0.003);
    dry_spine->AddElement(elCl,0.001);
    dry_spine->AddElement(elK,0.001);
    dry_spine->AddElement(elCa,0.133);

    // Dry rib
    G4Material* dry_rib = new G4Material( "DryRib",
                                            density = 1.92*g/cm3,
                                            numberofElements = 9);
    dry_rib->AddElement(elH,0.034);
    dry_rib->AddElement(elC,0.155);
    dry_rib->AddElement(elN,0.042);
    dry_rib->AddElement(elO,0.435);
    dry_rib->AddElement(elNa,0.001);
    dry_rib->AddElement(elMg,0.002);
    dry_rib->AddElement(elP,0.103);
    dry_rib->AddElement(elS,0.003);
    dry_rib->AddElement(elCa,0.225);


    // Adipose tissue
    G4Material* adiposeTissue = new G4Material( "AdiposeTissue",
                                               density = 0.95*g/cm3,
                                               numberofElements = 7);
    adiposeTissue->AddElement(elH,0.114);
    adiposeTissue->AddElement(elC,0.598);
    adiposeTissue->AddElement(elN,0.007);
    adiposeTissue->AddElement(elO,0.278);
    adiposeTissue->AddElement(elNa,0.001);
    adiposeTissue->AddElement(elS,0.001);
    adiposeTissue->AddElement(elCl,0.001);

    // Blood
    G4Material* blood = new G4Material( "Blood",
                                               density = 1.06*g/cm3,
                                               numberofElements = 10);
    blood->AddElement(elH,0.102);
    blood->AddElement(elC,0.11);
    blood->AddElement(elN,0.033);
    blood->AddElement(elO,0.745);
    blood->AddElement(elNa,0.001);
    blood->AddElement(elP,0.001);
    blood->AddElement(elS,0.002);
    blood->AddElement(elCl,0.003);
    blood->AddElement(elK,0.002);
    blood->AddElement(elFe,0.001);

    // Heart
    G4Material* heart = new G4Material( "Heart",
                                               density = 1.05*g/cm3,
                                               numberofElements = 9);
    heart->AddElement(elH,0.104);
    heart->AddElement(elC,0.139);
    heart->AddElement(elN,0.029);
    heart->AddElement(elO,0.718);
    heart->AddElement(elNa,0.001);
    heart->AddElement(elP,0.002);
    heart->AddElement(elS,0.002);
    heart->AddElement(elCl,0.002);
    heart->AddElement(elK,0.003);

    // Kidney
    G4Material* kidney = new G4Material( "Kidney",
                                               density = 1.05*g/cm3,
                                               numberofElements = 10);
    kidney->AddElement(elH,0.103);
    kidney->AddElement(elC,0.132);
    kidney->AddElement(elN,0.03);
    kidney->AddElement(elO,0.724);
    kidney->AddElement(elNa,0.002);
    kidney->AddElement(elP,0.002);
    kidney->AddElement(elS,0.002);
    kidney->AddElement(elCl,0.002);
    kidney->AddElement(elK,0.002);
    kidney->AddElement(elCa,0.001);

    // Liver
	G4Material* liver = new G4Material( "Liver",
									   density = 1.06*g/cm3,
									   numberofElements = 9);
	liver->AddElement(elH,0.102);
	liver->AddElement(elC,0.139);
	liver->AddElement(elN,0.030);
	liver->AddElement(elO,0.716);
	liver->AddElement(elNa,0.002);
	liver->AddElement(elP,0.003);
	liver->AddElement(elS,0.003);
	liver->AddElement(elCl,0.002);
	liver->AddElement(elK,0.003);

    // Lymph
	G4Material* lymph = new G4Material( "Lymph",
									   density = 1.03*g/cm3,
									   numberofElements = 7);
	lymph->AddElement(elH,0.108);
	lymph->AddElement(elC,0.041);
	lymph->AddElement(elN,0.011);
	lymph->AddElement(elO,0.832);
	lymph->AddElement(elNa,0.003);
	lymph->AddElement(elS,0.001);
	lymph->AddElement(elCl,0.004);

    // Pancreas
	G4Material* pancreas = new G4Material( "Pancreas",
									   density = 1.04*g/cm3,
									   numberofElements = 9);
	pancreas->AddElement(elH,0.106);
	pancreas->AddElement(elC,0.169);
	pancreas->AddElement(elN,0.022);
	pancreas->AddElement(elO,0.694);
	pancreas->AddElement(elNa,0.002);
	pancreas->AddElement(elP,0.002);
	pancreas->AddElement(elS,0.001);
	pancreas->AddElement(elCl,0.002);
	pancreas->AddElement(elK,0.002);

    // Intestine
	G4Material* intestine = new G4Material( "Intestine",
									   density = 1.03*g/cm3,
									   numberofElements = 9);
	intestine->AddElement(elH,0.106);
	intestine->AddElement(elC,0.115);
	intestine->AddElement(elN,0.022);
	intestine->AddElement(elO,0.751);
	intestine->AddElement(elNa,0.001);
	intestine->AddElement(elP,0.001);
	intestine->AddElement(elS,0.001);
	intestine->AddElement(elCl,0.002);
	intestine->AddElement(elK,0.001);

    // Skull
	G4Material* skull = new G4Material( "Skull",
									   density = 1.61*g/cm3,
									   numberofElements = 9);
	skull->AddElement(elH,0.05);
	skull->AddElement(elC,0.212);
	skull->AddElement(elN,0.04);
	skull->AddElement(elO,0.435);
	skull->AddElement(elNa,0.001);
	skull->AddElement(elMg,0.002);
	skull->AddElement(elP,0.081);
	skull->AddElement(elS,0.003);
	skull->AddElement(elCa,0.176);

    // Cartilage
	G4Material* cartilage = new G4Material( "Cartilage",
									   density = 1.10*g/cm3,
									   numberofElements = 8);
	cartilage->AddElement(elH,0.096);
	cartilage->AddElement(elC,0.099);
	cartilage->AddElement(elN,0.022);
	cartilage->AddElement(elO,0.744);
	cartilage->AddElement(elNa,0.005);
	cartilage->AddElement(elP,0.022);
	cartilage->AddElement(elS,0.009);
	cartilage->AddElement(elCl,0.003);

    // Brain
	G4Material* brain = new G4Material( "Brain",
									   density = 1.04*g/cm3,
									   numberofElements = 9);
	brain->AddElement(elH,0.107);
	brain->AddElement(elC,0.145);
	brain->AddElement(elN,0.022);
	brain->AddElement(elO,0.712);
	brain->AddElement(elNa,0.002);
	brain->AddElement(elP,0.004);
	brain->AddElement(elS,0.002);
	brain->AddElement(elCl,0.003);
	brain->AddElement(elK,0.003);

    // Spleen
	G4Material* spleen = new G4Material( "Spleen",
									   density = 1.06*g/cm3,
									   numberofElements = 9);
	spleen->AddElement(elH,0.103);
	spleen->AddElement(elC,0.113);
	spleen->AddElement(elN,0.032);
	spleen->AddElement(elO,0.741);
	spleen->AddElement(elNa,0.001);
	spleen->AddElement(elP,0.003);
	spleen->AddElement(elS,0.002);
	spleen->AddElement(elCl,0.002);
	spleen->AddElement(elK,0.003);

    // Iodine Blood
	G4Material* iodine_blood = new G4Material( "IodineBlood",
									   density = 1.09096*g/cm3,
									   numberofElements = 11);
	iodine_blood->AddElement(elH,0.101184);
	iodine_blood->AddElement(elC,0.10912);
	iodine_blood->AddElement(elN,0.032736);
	iodine_blood->AddElement(elO,0.73904);
	iodine_blood->AddElement(elNa,0.000992);
	iodine_blood->AddElement(elP,0.000992);
	iodine_blood->AddElement(elS,0.001984);
	iodine_blood->AddElement(elCl,0.002976);
	iodine_blood->AddElement(elK,0.001984);
	iodine_blood->AddElement(elFe,0.000992);
	iodine_blood->AddElement(elI,0.008);

    // Iron
    G4Material* iron = new G4Material( "Iron",
                                        density = 7.86 * g/cm3,
                                        numberofElements = 1 );
    iron->AddElement(elFe,1.0);

    // Pmma
    G4Material* pmma = new G4Material( "Pmma",
                                        density = 1.19 * g/cm3,
                                        numberofElements = 3 );
    pmma->AddElement(elH,0.08);
    pmma->AddElement(elC,0.6);
    pmma->AddElement(elO,0.32);

    // Aluminum
    G4Material* aluminum = new G4Material( "Aluminum",
                                        density = 2.6941 * g/cm3,
                                        numberofElements = 1 );
    aluminum->AddElement(elAl,1.0);

    // Titanium
    G4Material* titanium = new G4Material( "Titanium",
                                        density = 4.53 * g/cm3,
                                        numberofElements = 1 );
    titanium->AddElement(elTi,1.0);

    // Air
    G4Material* air = new G4Material( "Air",
    								0.001205*mg/cm3,
    								numberofElements = 4 );
    air->AddElement(elC, 0.000124);
    air->AddElement(elN, 0.755268);
    air->AddElement(elO, 0.231781);
    air->AddElement(elCl, 0.012827);

    // Graphite
    G4Material* graphite = new G4Material( "Graphite",
                                        density = 1.82 * g/cm3,
                                        numberofElements = 1 );
    graphite->AddElement(elC,1.0);

    // Lead
    G4Material* lead = new G4Material( "Lead",
                                        density = 11.34 * g/cm3,
                                        numberofElements = 1 );
    lead->AddElement(elPb,1.0);

    // Breast Mammary
	G4Material* breast_mammary = new G4Material( "breastMammary",
									   density = 1.02*g/cm3,
									   numberofElements = 8);
	breast_mammary->AddElement(elH,0.106);
	breast_mammary->AddElement(elC,0.332);
	breast_mammary->AddElement(elN,0.003);
	breast_mammary->AddElement(elO,0.527);
	breast_mammary->AddElement(elNa,0.001);
	breast_mammary->AddElement(elP,0.001);
	breast_mammary->AddElement(elS,0.002);
	breast_mammary->AddElement(elCl,0.001);

    // Skin
	G4Material* skin = new G4Material( "Skin",
									   density = 1.09*g/cm3,
									   numberofElements = 9);
	skin->AddElement(elH,0.10);
	skin->AddElement(elC,0.204);
	skin->AddElement(elN,0.0042);
	skin->AddElement(elO,0.645);
	skin->AddElement(elNa,0.002);
	skin->AddElement(elP,0.001);
	skin->AddElement(elS,0.002);
	skin->AddElement(elCl,0.003);
	skin->AddElement(elK,0.001);

	 // Iodine
	G4Material* iodine = new G4Material( "Iodine",
									   density = 4.933*g/cm3,
									   numberofElements = 1);
	iodine->AddElement(elI,1.0);

	 // eye_lens
	G4Material* eye_lens = new G4Material( "EyeLens",
									   density = 1.07*g/cm3,
									   numberofElements = 8);
	eye_lens->AddElement(elH,0.096);
	eye_lens->AddElement(elC,0.195);
	eye_lens->AddElement(elN,0.057);
	eye_lens->AddElement(elO,0.646);
	eye_lens->AddElement(elNa,0.001);
	eye_lens->AddElement(elP,0.001);
	eye_lens->AddElement(elS,0.003);
	eye_lens->AddElement(elCl,0.001);

	 // ovary
	G4Material* ovary = new G4Material( "Ovary",
									   density = 1.05*g/cm3,
									   numberofElements = 9);
	ovary->AddElement(elH,0.105);
	ovary->AddElement(elC,0.093);
	ovary->AddElement(elN,0.024);
	ovary->AddElement(elO,0.768);
	ovary->AddElement(elNa,0.002);
	ovary->AddElement(elP,0.002);
	ovary->AddElement(elS,0.002);
	ovary->AddElement(elCl,0.002);
	ovary->AddElement(elK,0.002);

	 // red_marrow
	G4Material* red_marrow = new G4Material( "RedMarrow",
									   density = 1.03*g/cm3,
									   numberofElements = 9);
	red_marrow->AddElement(elH,0.105);
	red_marrow->AddElement(elC,0.414);
	red_marrow->AddElement(elN,0.034);
	red_marrow->AddElement(elO,0.439);
	red_marrow->AddElement(elP,0.001);
	red_marrow->AddElement(elS,0.002);
	red_marrow->AddElement(elCl,0.002);
	red_marrow->AddElement(elK,0.002);
	red_marrow->AddElement(elFe,0.001);

	 // yellow_marrow
	G4Material* yellow_marrow = new G4Material( "YellowMarrow",
									   density = 0.98*g/cm3,
									   numberofElements = 7);
	yellow_marrow->AddElement(elH,0.115);
	yellow_marrow->AddElement(elC,0.644);
	yellow_marrow->AddElement(elN,0.007);
	yellow_marrow->AddElement(elO,0.231);
	yellow_marrow->AddElement(elNa,0.001);
	yellow_marrow->AddElement(elS,0.001);
	yellow_marrow->AddElement(elCl,0.001);

	 // testis
	G4Material* testis = new G4Material( "Testis",
									   density = 1.04*g/cm3,
									   numberofElements = 9);
	testis->AddElement(elH,0.106);
	testis->AddElement(elC,0.099);
	testis->AddElement(elN,0.02);
	testis->AddElement(elO,0.766);
	testis->AddElement(elNa,0.002);
	testis->AddElement(elP,0.001);
	testis->AddElement(elS,0.002);
	testis->AddElement(elCl,0.002);
	testis->AddElement(elK,0.002);

	 // thyroid
	G4Material* thyroid = new G4Material( "Thyroid",
									   density = 1.05*g/cm3,
									   numberofElements = 10);
	thyroid->AddElement(elH,0.104);
	thyroid->AddElement(elC,0.119);
	thyroid->AddElement(elN,0.024);
	thyroid->AddElement(elO,0.745);
	thyroid->AddElement(elNa,0.002);
	thyroid->AddElement(elP,0.001);
	thyroid->AddElement(elS,0.001);
	thyroid->AddElement(elCl,0.002);
	thyroid->AddElement(elK,0.001);
	thyroid->AddElement(elI,0.001);

	 // trabecular
	G4Material* trabecular = new G4Material( "trabecular",
									   density = 1.14*g/cm3,
									   numberofElements = 5);
	trabecular->AddElement(elH,0.079);
	trabecular->AddElement(elC,0.6379);
	trabecular->AddElement(elN,0.0423);
	trabecular->AddElement(elO,0.0988);
	trabecular->AddElement(elCa,0.142);

	 // bladder
	G4Material* bladder = new G4Material( "Bladder",
									   density = 1.04*g/cm3,
									   numberofElements = 9);
	bladder->AddElement(elH,0.105);
	bladder->AddElement(elC,0.096);
	bladder->AddElement(elN,0.026);
	bladder->AddElement(elO,0.761);
	bladder->AddElement(elNa,0.002);
	bladder->AddElement(elP,0.002);
	bladder->AddElement(elS,0.002);
	bladder->AddElement(elCl,0.003);
	bladder->AddElement(elK,0.003);

	//TODO: am I calculating the density correctly?
	 // dry_spine with water 0,3
	G4Material* dry_spine_water = new G4Material( "dry_spine_water",
									   density = (0.5*water->GetDensity() + 0.5*dry_spine->GetDensity())*g/cm3,
									   numberofElements = 2);
	dry_spine_water->AddMaterial(dry_spine,50.*perCent);
	dry_spine_water->AddMaterial(water,50.*perCent);

	 // dry_rib with water 0,4
	G4Material* dry_rib_water = new G4Material( "dry_rib_water",
									   density = (0.5*water->GetDensity() + 0.5*dry_rib->GetDensity())*g/cm3,
									   numberofElements = 2);
	dry_rib_water->AddMaterial(dry_rib,50.*perCent);
	dry_rib_water->AddMaterial(water,50.*perCent);

	 // skull with water 0,13
	G4Material* skull_water = new G4Material( "skull_water",
									   density = (0.5*water->GetDensity() + 0.5*skull->GetDensity())*g/cm3,
									   numberofElements = 2);
	skull_water->AddMaterial(skull,50.*perCent);
	skull_water->AddMaterial(water,50.*perCent);


    //----- Put the materials in a vector
    fMaterials.push_back(water);             //0
    fMaterials.push_back(muscle);            //1
    fMaterials.push_back(lung);              //2
    fMaterials.push_back(dry_spine);         //3
    fMaterials.push_back(dry_rib);			 //4
    fMaterials.push_back(adiposeTissue);	 //5
    fMaterials.push_back(blood);			 //6
    fMaterials.push_back(heart);			 //7
    fMaterials.push_back(kidney);			 //8
    fMaterials.push_back(liver);			 //9
    fMaterials.push_back(lymph);			 //10
    fMaterials.push_back(pancreas);			 //11
    fMaterials.push_back(intestine);		 //12
    fMaterials.push_back(skull);			 //13
    fMaterials.push_back(cartilage);		 //14
    fMaterials.push_back(brain);			 //15
    fMaterials.push_back(spleen);			 //16
    fMaterials.push_back(iodine_blood);		 //17
    fMaterials.push_back(iron);				 //18
    fMaterials.push_back(pmma);				 //19
    fMaterials.push_back(aluminum);			 //20
    fMaterials.push_back(titanium);			 //21
    fMaterials.push_back(air);				 //22
    fMaterials.push_back(graphite);			 //23
    fMaterials.push_back(lead);				 //24
    fMaterials.push_back(breast_mammary);	 //25
    fMaterials.push_back(skin);				 //26
    fMaterials.push_back(iodine);			 //27
    fMaterials.push_back(eye_lens);			 //28
    fMaterials.push_back(ovary);			 //29
    fMaterials.push_back(red_marrow);		 //30
    fMaterials.push_back(yellow_marrow);	 //31
    fMaterials.push_back(testis);			 //32
    fMaterials.push_back(thyroid);			 //33
    fMaterials.push_back(trabecular);		 //34
    fMaterials.push_back(bladder);			 //35
    fMaterials.push_back(dry_spine_water);	 //36
    fMaterials.push_back(dry_rib_water);	 //37
    fMaterials.push_back(skull_water);		 //38


}

void B1DetectorConstruction::ConstructSDandField()
{
	params parameters;

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
