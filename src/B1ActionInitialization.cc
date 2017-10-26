#include "B1ActionInitialization.hh"
#include "B1PrimaryGeneratorAction.hh"
#include "B1EventAction.hh"
#include "B1RunAction.hh"
#include "B1SteppingAction.hh"
#include "B1TrackingAction.hh"
#include "B1StackingAction.hh"

B1ActionInitialization::B1ActionInitialization()
 : G4VUserActionInitialization()
{}


B1ActionInitialization::~B1ActionInitialization()
{}


void B1ActionInitialization::BuildForMaster() const
{
	SetUserAction(new B1RunAction);
}


void B1ActionInitialization::Build() const
{
  params parameters;
  SetUserAction(new B1PrimaryGeneratorAction);
  SetUserAction(new B1RunAction);
  //SetUserAction(new B1EventAction);
  SetUserAction(new B1SteppingAction);
  SetUserAction(new B1TrackingAction);
  if (parameters.Bias.killElectrons == true) {
	  SetUserAction(new B1StackingAction);
  }
}  

