/*
 * B1EnergyDeposit.cc
 *
 *  Created on: Apr 22, 2017
 *      Author: adamgeva
 */

#include "B1EnergyDeposit.hh"
#include "params.hh"
#include "B1TrackInformation.hh"



B1EnergyDeposit::B1EnergyDeposit(G4String name, G4int type)
: G4PSEnergyDeposit(name)
{fscorerType =  type;}

B1EnergyDeposit::~B1EnergyDeposit()
{ }

G4int B1EnergyDeposit::GetIndex(G4Step* step){
	params parameters;

	G4StepPoint* preStepPoint = step->GetPreStepPoint();
	//entering the detector

	G4TouchableHistory* touchable = (G4TouchableHistory*)(preStepPoint->GetTouchable());
	G4int ReplicaNum0 = touchable->GetReplicaNumber(0);
	G4int ReplicaNum1 = touchable->GetReplicaNumber(1);
	//G4int ReplicaNum2 = touchable->GetReplicaNumber(2);
//10 is the number of bins
	return (ReplicaNum0 + ReplicaNum1*parameters.MyparamsGeometry.numberOfRows);

}

G4bool B1EnergyDeposit::ProcessHits(G4Step* aStep,G4TouchableHistory* touchable){
	//test: getting information from tracks
	G4Track* track = aStep->GetTrack();
	G4bool result = FALSE;
	G4VUserTrackInformation* info = track->GetUserInformation();
	B1TrackInformation* theInfo = (B1TrackInformation*)info;
	G4int totalNumOfInteractions = theInfo->GetNumberOfCompton() + theInfo->GetNumberOfRayl();
	//G4cout << "TrackId = " << track->GetTrackID() << " number of total interactions = " << totalNumOfInteractions << G4endl;
	if (fscorerType==1 || fscorerType==0){
		result = recordInteraction(aStep,touchable,totalNumOfInteractions,fscorerType);
	}
	else if(fscorerType==2){
		result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
	}
	else if(fscorerType==3){
		if ((theInfo->GetNumberOfCompton()<2) && (theInfo->GetNumberOfRayl()==0)){
			result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
			} else {
				result = FALSE;
			}
	}
	else if(fscorerType==4){
		if ((theInfo->GetNumberOfRayl()<2) && (theInfo->GetNumberOfCompton()==0)){
			result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
			} else {
				result = FALSE;
			}
	}
	return result;
}

G4bool B1EnergyDeposit::recordInteraction (G4Step* aStep,G4TouchableHistory* touchable, G4int totalNumOfInteractions, G4int i) {
	if (totalNumOfInteractions>i){//recording all photons that did not interact in the phantom - transmission
		return FALSE;
	} else {
		//G4cout << "recording" << i << G4endl;
		return G4PSEnergyDeposit::ProcessHits(aStep,touchable);
	}
}
