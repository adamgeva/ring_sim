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
#include "globalFunctions.hh"



B1SteppingAction::B1SteppingAction()
:G4UserSteppingAction()
{ }


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

	segment currSegmant;

	//we are not counting interaction that occur inside the detector
	if((procName == "compt" || procName == "biasWrapper(compt)") && startPhysicalName!="detectorPixelP") {
		//G4cout<<"We have Compton with prePhysical : " << startPhysicalName <<  G4endl;
		theInfo->AddCompton();

	}
	if((procName == "Rayl" || procName == "biasWrapper(Rayl)") && startPhysicalName!="detectorPixelP") {
		//G4cout<<"We have Rayl with prePhysical : " << startPhysicalName << G4endl;
		theInfo->AddRayl();
	}

	currSegmant.incidentEnergy = startPoint->GetKineticEnergy();
	currSegmant.scatteredEnergy = endPoint->GetKineticEnergy();
	currSegmant.pathLen = sqrt(pow((endPoint->GetPosition().x() - startPoint->GetPosition().x()),2) +
							   pow((endPoint->GetPosition().y() - startPoint->GetPosition().y()),2) +
							   pow((endPoint->GetPosition().z() - startPoint->GetPosition().z()),2));

	currSegmant.thetaScatter = angleBetweenVecs(startPoint->GetMomentumDirection(),endPoint->GetMomentumDirection());
	currSegmant.endingProcess = procName;

	//adding segment to list
	theInfo->AddSegment(currSegmant);

//	std::cout << "currSegmant.incidentEnergy" << currSegmant.incidentEnergy << " , currSegmant.scatteredEnergy " <<
//			currSegmant.scatteredEnergy << " , currSegmant.pathLen" << currSegmant.pathLen <<
//			" , currSegmant.thetaScatter" << currSegmant.thetaScatter << " , currSegmant.endingProcess" << currSegmant.endingProcess << std::endl;
//
//	std::cout << "startPoint" << startPoint->GetPosition() << " , endPoint" << endPoint->GetPosition() << std::endl;
//	std::cout << "procName" << procName << std::endl;


	//is this necessary?
	//theTrack->SetUserInformation(theInfo);

}





