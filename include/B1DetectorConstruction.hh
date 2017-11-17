#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "globals.hh"

#include <vector>

class G4VPhysicalVolume;
class G4LogicalVolume;

//defining struct for effective material
struct matProps {
	  G4double density;
	  G4double aEff;
	  G4double zEff;
};

// Detector construction class to define materials and geometry.
class B1DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    B1DetectorConstruction();
    virtual ~B1DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    matProps ReadMatProperties(const G4String& fname);
    virtual void ConstructSDandField();
    


  private:
    //logical detectors
    G4LogicalVolume* detectorPixelLV;
    std::vector<G4VisAttributes*> fVisAttributes;


};


#endif

