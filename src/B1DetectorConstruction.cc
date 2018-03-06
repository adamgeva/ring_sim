#include "B1DetectorConstruction.hh"
#include "myDetectorSD.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"

#include "G4SDManager.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4ios.hh"
#include "globalFunctions.hh"
#include "B1EnergyDeposit.hh"
#include "G4PSTrackLength.hh"

#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "B1BOptrMultiParticleChangeCrossSection.hh"
#include "B1BOptrFS.hh"
#include "B1BOptrFD.hh" //Forced Detection


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
  fMinX(0),
  fMinY(0),
  fMinZ(0), // minimum extension of voxels (position of wall)
  fMaxX(0),
  fMaxY(0),
  fMaxZ(0),
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

	//set tolerance
	G4GeometryManager::GetInstance()->SetWorldMaximumExtent(2.*WORLD_XY*cm);
	std::cout << "GetSurfaceTolerance() = " << G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()/mm << std::endl;
	std::cout << "GetAngularTolerance() = " << G4GeometryTolerance::GetInstance()->GetAngularTolerance()/mm << std::endl;
	std::cout << "GetRadialTolerance() = " << G4GeometryTolerance::GetInstance()->GetRadialTolerance()/mm << std::endl;


	// Get nist material manager
	G4NistManager* nist = G4NistManager::Instance();
	// Option to switch on/off checking of volumes overlaps
	G4bool checkOverlaps = true;

	// World
	// sizes are half size
	G4double world_sizeXY = WORLD_XY*cm;
	G4double world_sizeZ  = WORLD_Z*cm;
	G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

	G4Box* worldS = new G4Box("World",world_sizeXY, world_sizeXY, world_sizeZ);
	worldLV = new G4LogicalVolume(worldS, world_mat, "World");
	G4VPhysicalVolume* worldPHS = new G4PVPlacement(0, G4ThreeVector(),worldLV,"World",0,false,0,checkOverlaps);

	// detector - specs
	G4Material* detectorMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
	G4double detector_sizeX = DETECTOR_X*mm;
	G4double detector_sizeY = DETECTOR_Y*mm;
	G4double detector_sizeZ = DETECTOR_Z*mm;
	G4double dist = CENTER_TO_DET*mm + DETECTOR_Z*mm;

	// detector1
	G4Box* detectorS = new G4Box("detector",detector_sizeX, detector_sizeY, detector_sizeZ);
	G4LogicalVolume* detectorLV = new G4LogicalVolume(detectorS, detectorMat,"detector");

	//constant shifting
	G4double offset_u = OFFSET_U*mm;
	G4double offset_v = OFFSET_V*mm;

	G4int numOfItr = NUM_OF_DET_COLS;
	G4cout << "numOfItr" << numOfItr <<G4endl;
	for (G4int i=0;i<numOfItr;i++)
	{
		G4RotationMatrix* rotD = new G4RotationMatrix();
		rotD->rotateX(-M_PI/2);
		rotD->rotateY(M_PI/2);
		G4double y_shift = -detector_sizeX*(NUM_OF_DET_COLS -1 ) + 2*i*detector_sizeX + offset_u;
		//G4ThreeVector detectorPosUpdated = G4ThreeVector(cos(theta)*(dist), -X +j*2*X, sin(theta)*(dist));
		G4ThreeVector detectorPosUpdated = G4ThreeVector(dist,y_shift,offset_v);

		if (BUILD_DETECTORS == 1){
			new G4PVPlacement(rotD,detectorPosUpdated,detectorLV,"detector",worldLV,false,i,checkOverlaps);
		}
	}

	//Pixel
	G4Box* detectorPixelS = new G4Box("detectorCell",detector_sizeX, detector_sizeY/NUM_OF_DET_ROWS, detector_sizeZ);
	detectorPixelLV = new G4LogicalVolume(detectorPixelS, detectorMat,"detectorPixel");
	new G4PVReplica("detectorPixelP",detectorPixelLV,detectorLV,kYAxis,NUM_OF_DET_ROWS,2*detector_sizeY/NUM_OF_DET_ROWS);


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
	if (BUILD_PHANTOM == 1){
		//initialize materials
		if(CT_PHANTOM){
			//InitialisationOfMaterialsCT();
			//basic and wrond representation of GT materials
			InitialisationOfMaterialsCT_basic();
			ReadPhantomDataCT();

		} else{
			InitialisationOfMaterials();
			ReadPhantomData();
		}
		ConstructPhantomContainer();
		ConstructPhantom();
	}

	// User Limits

  // Set additional contraints on the track, with G4UserSpecialCuts
  //
   G4double maxStep=DBL_MAX, maxLength = DBL_MAX, maxTime = DBL_MAX, minEkin = PARTICLE_ENERGY*keV + 5*keV;
   detectorPixelLV->SetUserLimits(new G4UserLimits(maxStep,maxLength,maxTime,minEkin));
//   if (BUILD_PHANTOM == 1){
//	   fvoxel_logic->SetUserLimits(new G4UserLimits(maxStep,maxLength,maxTime,minEkin));
//   }

 //-------------------regions-------------------

//   G4Region* Container_region = new G4Region("ContainerRegion");
//   fContainer_logic->SetRegion(Container_region);
//   Container_region->AddRootLogicalVolume(fContainer_logic);



	//always return the physical World
	return worldPHS;
}

void B1DetectorConstruction::setContainerRotation(G4double delta){
	//sets the rotation of the phantom
	G4RotationMatrix* rot = new G4RotationMatrix();
	rot->rotateZ(delta);
	fContainer_phys->SetRotation(rot);
	G4RunManager::GetRunManager()->GeometryHasBeenModified();
	return;
}

void B1DetectorConstruction::ConstructPhantomContainer()
{

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

void B1DetectorConstruction::InitialisationOfMaterialsCT(){
	//initialization based on the materials from DICOM example
	G4String MaterialName[NUM_OF_BASE_MATERIALS] = {
			"G4_AIR",
			"G4_LUNG_ICRP",
			"G4_ADIPOSE_TISSUE_ICRP",
			"G4_WATER",
			"G4_MUSCLE_WITH_SUCROSE",
			"G4_B-100_BONE",
			"G4_BONE_COMPACT_ICRU",
			"G4_BONE_CORTICAL_ICRP",
			"G4_Fe"
	};
	//const G4MaterialTable* matTab = G4Material::GetMaterialTable();
	//iterate over the base materials

	//TODO: delete later
	std::ofstream output;
	std::string fileName = std::string(OUTPUT_DIR) + "/Dicom_base_materials.csv";
	output.open(fileName.c_str());

	for (G4int i = 0; i<NUM_OF_BASE_MATERIALS; i++){
		G4Material* material = 0;
		material = G4NistManager::Instance()->FindOrBuildMaterial(MaterialName[i]);
		thePhantomMaterialsOriginal[i] = material;


		const G4ElementVector* curr_element_vector = material->GetElementVector();
		const G4double* curr_frac_vector = material->GetFractionVector();
		G4int nElements = material->GetNumberOfElements();
		output << material->GetDensity()/(g/cm3) << ',';
		for (G4int el=0 ; el<nElements ; el++) {
			G4Element* el_i =  (*curr_element_vector)[el];
			G4String n = el_i->GetName();
			G4double ZZ = el_i->GetZ();
			G4double frac = curr_frac_vector[el];
			output << ZZ << ',' << frac << ',';
		}
		output << '\n';

	}
	output.close();

}

void B1DetectorConstruction::InitialisationOfMaterialsCT_basic()
{
    // Creating elements :
    G4double z, a, density;
    G4String name, symbol;
    G4int numberofElements;

    G4Element* elO = new G4Element( name = "Oxygen",
                                   symbol = "O",
                                   z = 8.0, a = 16.00  * g/mole );

    G4Element* elCa = new G4Element( name="Calcium",
                                    symbol = "Ca",
                                    z = 20.0, a = 40.078* g/mole );


//*******************************************************************************************************************************************************
	//G4String fname = FILE_MATERIALS_GT_BASIC;
    G4String fname = FILE_MATERIALS;
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	if( !fin.is_open() ) {
	   G4Exception("Can't read materials_file",
					"",
					FatalErrorInArgument,
					G4String("File not found " + fname ).c_str());
	  }
	//pointers to materials
	G4Material* material;
	G4String MaterialName[NUM_OF_BASE_MATERIALS];
	//iterate over the base materials
	G4int mat = 0;
	G4int numOfEl = NUM_OF_ELEMENTS;
	for (mat = 0; mat<NUM_OF_BASE_MATERIALS; mat ++){
		if( fin.eof() ) break;
		// material - place in an array
		MaterialName[mat] = "mat" + IntToString(mat);
		G4double* fracs = new G4double[numOfEl];
		// read density
		//fin >> density;
		//if (mat==0) {density=1;}
		//else {density = 0.000000000000000001;}
		material = new G4Material( name = MaterialName[mat],
											   1*g/cm3,
											   numberofElements = 2);
		std::cout << "material number: " << mat << " rho: " << density << " fractions: ";
		// read fractions
		for (G4int i=0; i<numOfEl; i++){
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
		material->AddElement(elO,fracs[2]);
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
		material->AddElement(elCa,fracs[4]);
//		material[voxel]->AddElement(elSc,fracs[19]);
//		material[voxel]->AddElement(elTi,fracs[20]);
//		material[voxel]->AddElement(elV,fracs[21]);
//		material[voxel]->AddElement(elCr,fracs[22]);
//		material[voxel]->AddElement(elMn,fracs[23]);
//		material[voxel]->AddElement(elFe,fracs[24]);
//		material[voxel]->AddElement(elI,fracs[25]);
//		material[voxel]->AddElement(elPb,fracs[26]);

		//add material to fMaterials
		thePhantomMaterialsOriginal[mat] = material;
		delete fracs;
	}
}

void B1DetectorConstruction::InitialisationOfMaterials()
{
    // Creating elements :
    G4double z, a, density;
    G4String name, symbol;
    G4int numberofElements;
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
	G4String fname = FILE_MATERIALS;
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	if( !fin.is_open() ) {
	   G4Exception("Can't read materials_file",
					"",
					FatalErrorInArgument,
					G4String("File not found " + fname ).c_str());
	  }
	//pointers to materials
	G4Material* material;
	G4String MaterialName[NUM_OF_BASE_MATERIALS];
	//iterate over the base materials
	G4int mat = 0;
	for (mat = 0; mat<NUM_OF_BASE_MATERIALS; mat ++){
		if( fin.eof() ) break;
		// material - place in an array
		MaterialName[mat] = "mat" + IntToString(mat);
		G4double fracs[NUM_OF_ELEMENTS];
		// read density

		material = new G4Material( name = MaterialName[mat],
											   density = density*g/cm3,
											   numberofElements = 2);
		std::cout << "material number: " << mat << " rho: " << 1 << " fractions: ";
		// read fractions
		for (G4int i=0; i<NUM_OF_ELEMENTS; i++){
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
		material->AddElement(elO,fracs[2]);
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
		material->AddElement(elCa,fracs[4]);
//		material[voxel]->AddElement(elSc,fracs[19]);
//		material[voxel]->AddElement(elTi,fracs[20]);
//		material[voxel]->AddElement(elV,fracs[21]);
//		material[voxel]->AddElement(elCr,fracs[22]);
//		material[voxel]->AddElement(elMn,fracs[23]);
//		material[voxel]->AddElement(elFe,fracs[24]);
//		material[voxel]->AddElement(elI,fracs[25]);
//		material[voxel]->AddElement(elPb,fracs[26]);

		//add material to fMaterials
		fBaseMaterials[mat] = material;
	}
}


void B1DetectorConstruction::ReadPhantomData()
{
    // initiallize fMateIDs
	fMateIDs = new size_t[NUM_OF_VOXELS];

    G4String fname = FILE_VOXEL_TO_MATERIALS;
	std::ifstream fin(fname.c_str(), std::ios_base::in);
	if( !fin.is_open() ) {
	   G4Exception("Can't read voxels_to_materials_file",
					"",
					FatalErrorInArgument,
					G4String("File not found " + fname ).c_str());
	}

    G4double index;
    G4double newRho;
	G4Material* newMaterial;
	G4String baseMaterialName;
	G4String newMaterialName;

	for( G4int iz = 0; iz < NUM_OF_Z_SLICES; iz++ ) {
		for( G4int iy = 0; iy < NUM_OF_VOXELS_Y; iy++ ) {
			for( G4int ix = 0; ix < NUM_OF_VOXELS_X; ix++ ) {
				if( fin.eof() ) break;
				G4int voxel = ix + (iy)*NUM_OF_VOXELS_X + (iz)*NUM_OF_VOXELS_X*NUM_OF_VOXELS_Y;
				fin >> index;
				fin >> newRho;
				baseMaterialName = fBaseMaterials[G4int(index)]->GetName();
				//const G4double* fracs = fBaseMaterials[G4int(index)]->GetFractionVector();
				//std::cout << "voxel number: " << voxel << " rho: " << newRho << " atom1 fraction: " << fracs[0] << " atom 2 fraction: " << fracs[1] << "\n";
				newMaterialName = "voxelMat" + IntToString(voxel);
				newMaterial = G4NistManager::Instance()->
				  BuildMaterialWithNewDensity(newMaterialName,baseMaterialName,newRho*g/cm3);

				fMaterials.push_back(newMaterial);
				fMateIDs[voxel] = voxel;
			}
		}
	}
}


void B1DetectorConstruction::ReadPhantomDataCT()
{

  //---- Extract number of voxels and voxel dimensions
  fNVoxelX = NUM_OF_VOXELS_X;
  fNVoxelY = NUM_OF_VOXELS_Y;
  fNVoxelZ = NUM_OF_Z_SLICES;

  fVoxelHalfDimX = VOXEL_HALF_X*mm;
  fVoxelHalfDimY = VOXEL_HALF_Y*mm;
  fVoxelHalfDimZ = VOXEL_HALF_Z*mm;

  fMateIDs = new size_t[fNVoxelX*fNVoxelY*fNVoxelZ];
  for( G4int iz = 0; iz < fNVoxelZ; iz++ ) {
	  //FILE_VOXEL_TO_MATERIALS_TEST
    G4String fileName_id = (G4String)FILE_VOXEL_TO_MATERIALS_Y + "id" + IntToString(iz) + ".dat";
    std::ifstream fin_id(fileName_id);
    std::cout << "reading file: " << fileName_id << std::endl;
    for( G4int iy = 0; iy < fNVoxelY; iy++ ) {
      for( G4int ix = 0; ix < fNVoxelX; ix++ ) {
        G4int mateID;
        fin_id >> mateID;
        G4int nnew = ix + (iy)*fNVoxelX + (iz)*fNVoxelX*fNVoxelY;
        if( mateID < 0 || mateID >= NUM_OF_BASE_MATERIALS ) {
          G4Exception("GmReadPhantomG4Geometry::ReadPhantomData",
                      "Wrong index in phantom file",
                      FatalException,
                      G4String("It should be between 0 and "
                               + G4UIcommand::ConvertToString(NUM_OF_BASE_MATERIALS-1)
                               + ", while it is "
                               + G4UIcommand::ConvertToString(mateID)).c_str());
        }
        fMateIDs[nnew] = mateID;
      }
    }
    fin_id.close();
  }

  ReadVoxelDensities( );

}


void B1DetectorConstruction::ReadVoxelDensities( )
{

	G4String stemp;
	  std::map<G4int, std::pair<G4double,G4double> > densiMinMax;
	  std::map<G4int, std::pair<G4double,G4double> >::iterator mpite;
	  for( size_t ii = 0; ii < thePhantomMaterialsOriginal.size(); ii++ ){
	    densiMinMax[ii] = std::pair<G4double,G4double>(DBL_MAX,-DBL_MAX);
	  }

	  //char* part = getenv( "DICOM_CHANGE_MATERIAL_DENSITY" );
	  G4double densityDiff = -1.;
	  //if( part ) densityDiff = G4UIcommand::ConvertToDouble(part);

	  std::map<G4int,G4double> densityDiffs;
	  for( size_t ii = 0; ii < thePhantomMaterialsOriginal.size(); ii++ ){
	    densityDiffs[ii] = densityDiff; //currently all materials with same step
	  }
	  //  densityDiffs[0] = 0.0001; //air


	  //--- Calculate the average material density for each material/density bin
	  std::map< std::pair<G4Material*,G4int>, matInfo* > newMateDens;

	  //---- Read the material densities
	  G4double dens;
	  for( G4int iz = 0; iz < fNVoxelZ; iz++ ) {
		G4String fileName_dens = (G4String)FILE_VOXEL_TO_MATERIALS_Y + "dens" + IntToString(iz) + ".dat";
		std::ifstream fin_dens(fileName_dens);
		std::cout << "reading file: " << fileName_dens << std::endl;
	    for( G4int iy = 0; iy < fNVoxelY; iy++ ) {
	      for( G4int ix = 0; ix < fNVoxelX; ix++ ) {
	    	fin_dens >> dens;
	        //        G4cout << ix << " " << iy << " " << iz << " density " << dens << G4endl;
	        G4int copyNo = ix + (iy)*fNVoxelX + (iz)*fNVoxelX*fNVoxelY;

	        if( densityDiff != -1. ) continue;

	        //--- store the minimum and maximum density for each material (just for printing)
	        mpite = densiMinMax.find( fMateIDs[copyNo] );
	        if( dens < (*mpite).second.first ) (*mpite).second.first = dens;
	        if( dens > (*mpite).second.second ) (*mpite).second.second = dens;
	        //--- Get material from original list of material in file
	        int mateID = fMateIDs[copyNo];
	        std::map<G4int,G4Material*>::const_iterator imite =
	         thePhantomMaterialsOriginal.find(mateID);
	        //        G4cout << copyNo << " mateID " << mateID << G4endl;
	        //--- Check if density is equal to the original material density
	        if( std::fabs(dens - (*imite).second->GetDensity()/CLHEP::g*CLHEP::cm3 ) < 1.e-9 ) continue;

	        //--- Build material name with thePhantomMaterialsOriginal name + density
	        //        float densityBin = densityDiffs[mateID] * (G4int(dens/densityDiffs[mateID])+0.5);
	        G4int densityBin = (G4int(dens/densityDiffs[mateID]));

	        G4String mateName = (*imite).second->GetName()+G4UIcommand::ConvertToString(densityBin);
	        //--- Look if it is the first voxel with this material/densityBin
	        std::pair<G4Material*,G4int> matdens((*imite).second, densityBin );

	        std::map< std::pair<G4Material*,G4int>, matInfo* >::iterator mppite =
	         newMateDens.find( matdens );
	        if( mppite != newMateDens.end() ){
	          matInfo* mi = (*mppite).second;
	          mi->fSumdens += dens;
	          mi->fNvoxels++;
	          fMateIDs[copyNo] = thePhantomMaterialsOriginal.size()-1 + mi->fId;
	        } else {
	          matInfo* mi = new matInfo;
	          mi->fSumdens = dens;
	          mi->fNvoxels = 1;
	          mi->fId = newMateDens.size()+1;
	          newMateDens[matdens] = mi;
	          fMateIDs[copyNo] = thePhantomMaterialsOriginal.size()-1 + mi->fId;
	        }
	      }
	    }
	    fin_dens.close();
	  }

	  if( densityDiff != -1. ) {
	    for( mpite = densiMinMax.begin(); mpite != densiMinMax.end(); mpite++ ){

	    }
	  }

	  //----- Build the list of phantom materials that go to Parameterisation
	  //--- Add original materials
	  std::map<G4int,G4Material*>::const_iterator mimite;
	  for( mimite = thePhantomMaterialsOriginal.begin(); mimite != thePhantomMaterialsOriginal.end();
	   mimite++ ){
	    fMaterials.push_back( (*mimite).second );
	  }
	  //
	  //---- Build and add new materials

	  //G4int sDens = newMateDens.sisze();
	  std::map< G4int, std::pair<G4Material*,G4int> > sorted_map_mat;
	  std::map< std::pair<G4Material*,G4int>, matInfo* >::iterator mppite;

	  for( mppite= newMateDens.begin(); mppite != newMateDens.end(); mppite++ ){
		  G4int idd = (*mppite).second->fId;
		  sorted_map_mat[idd] = (*mppite).first;
	  }

	  std::map< G4int, std::pair<G4Material*,G4int> >::iterator iterator_sorted;
	  for( iterator_sorted= sorted_map_mat.begin(); iterator_sorted != sorted_map_mat.end(); iterator_sorted++ ){
		  std::pair<G4Material*,G4int> key = (*iterator_sorted).second;
		  mppite = newMateDens.find( key );

		  G4double averdens = (*mppite).second->fSumdens/(*mppite).second->fNvoxels;
		  G4double saverdens = G4int(1000.001*averdens)/1000.;

		  G4String mateName = ((*mppite).first).first->GetName() + "_"
		      + G4UIcommand::ConvertToString(saverdens);
		  fMaterials.push_back( BuildMaterialWithChangingDensity(
		      (*mppite).first.first, averdens, mateName ) );
	  }

/*
	  for( mppite= newMateDens.begin(); mppite != newMateDens.end(); mppite++ ){
	    G4double averdens = (*mppite).second->fSumdens/(*mppite).second->fNvoxels;
	    G4double saverdens = G4int(1000.001*averdens)/1000.;

	      G4String mateName = ((*mppite).first).first->GetName() + "_"
	       + G4UIcommand::ConvertToString(saverdens);
	    fMaterials.push_back( BuildMaterialWithChangingDensity(
	     (*mppite).first.first, averdens, mateName ) );
	  }
*/
}

G4Material* B1DetectorConstruction::BuildMaterialWithChangingDensity(
           const G4Material* origMate, float density, G4String newMateName )
{
  //----- Copy original material, but with new density
  G4int nelem = origMate->GetNumberOfElements();
  G4Material* mate = new G4Material( newMateName, density*g/cm3, nelem,
                                     kStateUndefined, STP_Temperature );

  for( G4int ii = 0; ii < nelem; ii++ ){
    G4double frac = origMate->GetFractionVector()[ii];
    G4Element* elem = const_cast<G4Element*>(origMate->GetElement(ii));
    mate->AddElement( elem, frac );
  }

  return mate;
}

void B1DetectorConstruction::ConstructSDandField()
{
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
if (BUILD_PHANTOM==1){
	  B1BOptrFD* FDOptr =  new B1BOptrFD("gamma","FDOperator");
	  FDOptr->AttachTo(fvoxel_logic); //maybe should be attache to fcontainer?
	  //FSOptr->AttachTo(logicWorld);
	  //comptLEOptr->AttachTo(logicTestBone);
	  G4cout << " Attaching biasing operator " << FDOptr->GetName()
			 << " to logical volume " << fvoxel_logic->GetName()
			 << G4endl;
}
//  G4BOptrForceCollision* OptrForceCollision =  new G4BOptrForceCollision("gamma","forceCollision");
//  OptrForceCollision->AttachTo(logicTest);
//  //OptrForceCollision->AttachTo(logicTestBone);


	// sensitive detectors
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	//detector1 - SD
	//detector2 - Scorer
	//creating my sensitive detector and adding it to the SD manager - the data will be saved in histograms only if record hist is on
	if (RECORD_HIST == 1){
		G4String SDname;
		G4VSensitiveDetector* detector1 = new myDetectorSD(SDname="/detector1");
		SDman->AddNewDetector(detector1);
		//attaching my sensitive detector to the detector logical element
		SetSensitiveDetector(detectorPixelLV,detector1);
	}
	//creating scorer for detector
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





