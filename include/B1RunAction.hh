#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "B1EnergyDeposit.hh"
#include "G4Accumulable.hh"
#include "B1Accumulable.hh"
#include "B1DetectorConstruction.hh"

class G4Run;

// Run action class

class B1RunAction : public G4UserRunAction
{
  public:
    B1RunAction(B1DetectorConstruction* detectorConstruction);
    virtual ~B1RunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);
    G4Run* GenerateRun();
    //virtual void   EndOfRunAction();

  private:
    B1EnergyDeposit* fMyEnergyDeposit;
    //TODO: accumulable need to change to map
    //B1Accumulable fGradientAccumulable;
    B1DetectorConstruction* fdetectorConstruction;

};


#endif

