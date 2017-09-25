#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "globals.hh"

#include <vector>

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
    


  private:
    //logical detectors
    G4LogicalVolume* detectorPixelLV;
    std::vector<G4VisAttributes*> fVisAttributes;


};


#endif

