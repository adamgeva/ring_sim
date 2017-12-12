/*
 * B1EnergyDeposit.cc
 *
 *  Created on: Apr 22, 2017
 *      Author: adamgeva
 */

#include "B1EnergyDeposit.hh"
#include "params.hh"
#include "B1TrackInformation.hh"
#include "globalFunctions.hh"
#include "G4RunManager.hh"

extern B1EnergyDeposit* detectorsArray[NUM_OF_THREADS];


B1EnergyDeposit::B1EnergyDeposit(G4String name, G4int type)
: G4PSEnergyDeposit(name)
{
	params parameters;
	fscorerType =  type;
	if (type==0){ //randomly pick the 0 scorer
		G4int threadID = G4Threading::G4GetThreadId();
		if (threadID==-1) { //single threaded mode
			detectorsArray[0] = this;
		}
		else
		{
			detectorsArray[threadID+1] = this;
		}

	}

}

B1EnergyDeposit::~B1EnergyDeposit()
{

}

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
	params parameters;

	//test: getting information from tracks
	G4Track* track = aStep->GetTrack();
//	G4double trackWeight = track->GetWeight();


	//G4cout << "We have a hit, Weight: " << track->GetWeight() << G4endl;
//	G4String creatorProcessName = track->GetCreatorModelName();
//	G4int ParentID = track->GetParentID();
//	G4int ID = track->GetTrackID();
//	G4String type = track->GetDefinition()->GetParticleType();

	G4bool result = FALSE;
	G4VUserTrackInformation* info = track->GetUserInformation();
	B1TrackInformation* theInfo = (B1TrackInformation*)info;
	G4int totalNumOfInteractions = theInfo->GetNumberOfCompton() + theInfo->GetNumberOfRayl();

	//printing debug info:
//	G4cout << "track ID:" << track->GetTrackID() << G4endl;
//	G4cout << "track parent ID:" << track->GetParentID() << G4endl;
//	G4cout << "num of compt:" << theInfo->GetNumberOfCompton() << G4endl;
//	G4cout << "num of rayl:" << theInfo->GetNumberOfRayl() << G4endl;

	//very rare case in biasing - in case the photon underwent an interaction but wasnt split even tho it should have, example:
	//a photon hits the detector, potoelectric absorption and the an emittion of a new photon (ID 1 for example) then this photon undergoes Rayl in the phantom and arrives at the detector with weight 1.
	//TODO: check if correct!
	//not recording the main photons - only the virtuals
	if ((parameters.Myparams.onOffBiasing==1) && (track->GetParentID()==0)) {
		result = FALSE;
	}
	//G4cout << "TrackId = " << track->GetTrackID() << " number of total interactions = " << totalNumOfInteractions << G4endl;
	else if (fscorerType==1 || fscorerType==0){
		result = recordInteraction(aStep,touchable,totalNumOfInteractions,fscorerType);
	}
	else if(fscorerType==2){
		result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
	}
	else if(fscorerType==3){
		if ((theInfo->GetNumberOfCompton()<2) && (theInfo->GetNumberOfRayl()==0))
		{
			result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
		}
		else
		{
			result = FALSE;
		}
	}
	else if(fscorerType==4){
		if ((theInfo->GetNumberOfRayl()<2) && (theInfo->GetNumberOfCompton()==0)){
			result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
		}
		else
		{
			result = FALSE;
		}
	}

	if(fscorerType==0){
	    G4StepPoint* preStepPoint = aStep->GetPreStepPoint();
    	G4TouchableHistory* touchable2 = (G4TouchableHistory*)(preStepPoint->GetTouchable());
		G4int ReplicaNum1 = touchable2->GetReplicaNumber(1);
	   	while (!theInfo->fpathLogList.empty()){
	   		segment seg = theInfo->fpathLogList.front();
	   		theInfo->fpathLogList.pop_front();
	   		//TODO: needs to be generalized - maybe there will be another process once!
   			outputPathsFile << seg.voxel << "," << 0 << "," << seg.incidentEnergy/keV << "," << seg.pathLen << ",";
	   		G4String enddd = seg.endingProcess;
	   		if (seg.endingProcess == "compt"){
	   			outputPathsFile << seg.voxel << "," << 1 << "," << seg.incidentEnergy/keV << "," << seg.scatteredEnergy/keV << ","; //<< seg.incidentEnergy/keV << "," << seg.scatteredEnergy/keV << ",";
	   		}
	   	}
	   	outputPathsFile << ReplicaNum1 << "," << -100 << "\n";
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

void B1EnergyDeposit::openFile(G4int threadNum,G4int runNum){
		std::string fileName =  "../run_outputs/" + IntToString(runNum) + "run" + IntToString(threadNum) + "paths.csv";
		outputPathsFile.open(fileName.c_str());
		//G4cout << "created file: " << fileName <<G4endl;
}

void B1EnergyDeposit::writeFile() {
	if (fscorerType==0){ // safty only
		//G4cout << "closing file: " <<G4endl;
		outputPathsFile.close();
	}
}


