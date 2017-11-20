/*
 * B1TrackInformation.cc
 *
 *  Created on: Apr 23, 2017
 *      Author: adamgeva
 */


#include "B1TrackInformation.hh"
#include "G4ios.hh"

G4ThreadLocal G4Allocator<B1TrackInformation>* aTrackInformationAllocator=0;

B1TrackInformation::B1TrackInformation()
{
    originalTrackID = 0;
    particleDefinition = 0;
    originalPosition = G4ThreeVector(0.,0.,0.);
    originalMomentum = G4ThreeVector(0.,0.,0.);
    originalEnergy = 0.;
    originalTime = 0.;
    numberOfCompton = 0;
    numberOfRayl = 0;

}

B1TrackInformation::B1TrackInformation(const G4Track* aTrack)
{
    originalTrackID = aTrack->GetTrackID();
    particleDefinition = aTrack->GetDefinition();
    originalPosition = aTrack->GetPosition();
    originalMomentum = aTrack->GetMomentum();
    originalEnergy = aTrack->GetTotalEnergy();
    originalTime = aTrack->GetGlobalTime();
    numberOfCompton = 0;
    numberOfRayl = 0;

    //creating initial segment - infact this is not yet a segment
    segment initialSegment;
    initialSegment.pathLen = -1;
    initialSegment.incidentEnergy = aTrack->GetKineticEnergy();
    initialSegment.scatteredEnergy = -1;
    initialSegment.thetaScatter = -1;
    initialSegment.endingProcess = "start";
    // adding to path log
    fpathLogList.push_back(initialSegment);
}

B1TrackInformation::B1TrackInformation(const B1TrackInformation* aTrackInfo)
{

    originalTrackID = aTrackInfo->originalTrackID;
    particleDefinition = aTrackInfo->particleDefinition;
    originalPosition = aTrackInfo->originalPosition;
    originalMomentum = aTrackInfo->originalMomentum;
    originalEnergy = aTrackInfo->originalEnergy;
    originalTime = aTrackInfo->originalTime;
    //secondary particles recieve the parent's compton and rayl num of interactions
    numberOfCompton = aTrackInfo->GetNumberOfCompton();
    numberOfRayl = aTrackInfo->GetNumberOfRayl();
    //TODO: !!!
    //path log continues! but neet to account for weigths - they should be collected in segment
    fpathLogList = aTrackInfo->fpathLogList;
}

B1TrackInformation::~B1TrackInformation(){
	//no need to delete list because it is done automatically when the photon dies
	}

void B1TrackInformation::Print() const
{
    G4cout
     << "Original track ID " << originalTrackID
     << " originalPosition " << originalPosition
     << " originalMomentum " << originalMomentum
     << " originalEnergy " << originalEnergy
     << " numberOfCompton " << numberOfCompton
     << " numberOfRayl " << numberOfRayl << G4endl;

}


