#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "B1EnergyDeposit.hh"
#include "G4Accumulable.hh"
#include "B1Accumulable.hh"


class G4Run;

// Run action class

class B1RunAction : public G4UserRunAction
{
  public:
    B1RunAction();
    virtual ~B1RunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);
    G4Run* GenerateRun();
    //virtual void   EndOfRunAction();

  private:
    B1EnergyDeposit* fMyEnergyDeposit;
    // G4Accumulable<G4double> fTestAccum = G4Accumulable<G4double>("hey",0.0);
    B1Accumulable fGradientAccumulable;
};


#endif

