#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "globals.hh"
class G4Material;

#include <vector>
#include <set>
#include <map>

class G4VPhysicalVolume;
class G4LogicalVolume;

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
    std::vector<G4Material*> fOriginalMaterials;  // list of original materials
    //TODO: do we need all of that???
	std::vector<G4Material*> fMaterials;
	// list of new materials created to distinguish different density
	//  voxels that have the same original materials
	size_t* fMateIDs; // index of material of each voxel
	//unsigned int* fMateIDs; // index of material of each voxel

  private:
    //logical detectors
    G4LogicalVolume* detectorPixelLV;
    std::vector<G4VisAttributes*> fVisAttributes;

};


#endif

