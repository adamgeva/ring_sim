/*
 * B1SteppingAction.cc
 *
 *  Created on: Apr 23, 2017
 *      Author: adamgeva
 */

#include "B1SteppingAction.hh"
#include "G4VUserTrackInformation.hh"
#include "B1TrackInformation.hh"
#include "G4RunManager.hh"
#include "G4TransportationManager.hh"
#include "params.hh"

B1SteppingAction::B1SteppingAction()
:G4UserSteppingAction()
{
	params parameters;
	if (parameters.Myparams.G4navigatorVerbos==0){
		//suppressing navigator msgs
		G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->SetPushVerbosity(0);
	}
}



B1SteppingAction::~B1SteppingAction()
{ }


void B1SteppingAction::UserSteppingAction(const G4Step* aStep)
{
	G4StepPoint* endPoint = aStep->GetPostStepPoint();
	G4StepPoint* startPoint = aStep->GetPreStepPoint();

	G4String procName = endPoint->GetProcessDefinedStep()->GetProcessName();
	G4Track* theTrack = aStep->GetTrack();

	G4TouchableHistory* startTouchable = (G4TouchableHistory*)(startPoint->GetTouchable());

	G4String startPhysicalName = startTouchable->GetVolume()->GetName();


	G4VUserTrackInformation* info = theTrack->GetUserInformation();
	B1TrackInformation* theInfo = (B1TrackInformation*)info;

	//we are not counting interaction that occur inside the detector
	if(procName == "compt" && startPhysicalName!="detectorPixelP") {
		//G4cout<<"We have Compton with prePhysical : " << startPhysicalName <<  G4endl;
		theInfo->AddCompton();

	}
	if(procName == "Rayl" && startPhysicalName!="detectorPixelP") {
		//G4cout<<"We have Rayl with prePhysical : " << startPhysicalName << G4endl;
		theInfo->AddRayl();

	}

	//is this necessary?
	//theTrack->SetUserInformation(theInfo);

}





