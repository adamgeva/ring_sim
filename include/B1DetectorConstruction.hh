#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "globals.hh"
#include "G4Box.hh"
class G4Material;

#include <vector>
#include <set>
#include <map>

class G4VPhysicalVolume;
class G4LogicalVolume;

struct materialIDs {
  G4int mat1ID;
  G4int mat2ID;
};

// Detector construction class to define materials and geometry.
class B1DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    B1DetectorConstruction();
    virtual ~B1DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();
    
  protected:
    // create the original materials
    void InitialisationOfMaterials();
    void InitialisationOfMaterialsMap();
    void ReadPhantomData();
    void ReadPhantomDataFile(const G4String& fname,G4int sliceNumber);
    // read one of the DICOM files describing the phantom (usually one per Z slice).
    //  Build a DicomPhantomZSliceHeader for each file
    void ConstructPhantomContainer();
    virtual void ConstructPhantom() = 0;
	// construct the phantom volumes.
	//  This method should be implemented for each of the derived classes

  protected:
    G4int fNoFiles; // number of Phantom files- each file is one Z slice
    G4Box* fContainer_solid;
   	G4LogicalVolume* fContainer_logic;
   	G4VPhysicalVolume* fContainer_phys;
   	std::vector<G4Material*> fMaterials;

   	size_t* fMateIDs; // index of material of each voxel - this array is in the size of the number of voxels


   	G4int fNVoxelX, fNVoxelY, fNVoxelZ;
	G4double fVoxelHalfDimX, fVoxelHalfDimY, fVoxelHalfDimZ;
//	G4double fMinX,fMinY,fMinZ; // minimum extension of voxels (position of wall)
//	G4double fMaxX,fMaxY,fMaxZ; // maximum extension of voxels (position of wall)

	//TODO: what is this used for?
	std::map<G4int,G4Material*> thePhantomMaterialsOriginal;
	// map numberOfMaterial to G4Material. They are the list of materials as built from .geom file


	std::map<G4int,materialIDs> intensityToMateID;
	// maps the intensity value in every voxel to its material ID

  private:
	G4LogicalVolume* worldLV;
	//logical detectors
    G4LogicalVolume* detectorPixelLV;
    std::vector<G4VisAttributes*> fVisAttributes;


};


#endif

