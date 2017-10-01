#include "B1BOptnRaylSplitting.hh"
#include "B1BOptrFS.hh"
#include <vector>
#include "globalFunctions.hh"

#include "G4BiasingProcessInterface.hh"
#include "G4ParticleChangeForGamma.hh"
#include "B1BOptrFSTrackData.hh"
#include "G4VUserTrackInformation.hh"
#include "B1TrackInformation.hh"
#include "G4TrackStatus.hh"
#include "params.hh"
#include <math.h>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1BOptnRaylSplitting::B1BOptnRaylSplitting(G4String name)
: G4VBiasingOperation(name),
  fSplittingFactorNp(1),
  fSplittingFactorNs(1),
  fParticleChange()
{
  // -- get ID for FS:
  fFSModelID = G4PhysicsModelCatalog::Register("GenBiasFS");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1BOptnRaylSplitting::~B1BOptnRaylSplitting()
{
}

//G4Track* B1BOptnRaylSplitting::GetSplitTrack()
//{
//	if (!fsplitTracksVector.empty()) {
//		G4Track* track = fsplitTracksVector.back();
//		fsplitTracksVector.pop_back();
//		return track;
//	}
//	return nullptr;
//}

G4VParticleChange*
B1BOptnRaylSplitting::
ApplyFinalStateBiasing( const G4BiasingProcessInterface* callingProcess,
                        const G4Track*                            track,
                        const G4Step*                              step,
                        G4bool&                                         )
{

  params parameters;

  //fetch track data and split factor
  B1BOptrFSTrackData* AuxTrackData = (B1BOptrFSTrackData*)(track->GetAuxiliaryTrackInformation(fFSModelID));
  G4int splitFactor;
  if (AuxTrackData->IsPrimary())
  {
	  splitFactor = fSplittingFactorNp;
  }
  else
  {
	  splitFactor = fSplittingFactorNs;
  }

  // -- Collect Rayl. process (wrapped process) final state:
  G4VParticleChange* processFinalState =
    callingProcess->GetWrappedProcess()->PostStepDoIt(*track, *step);

  // -- if no splitting requested, let the Raylon. process to return directly its
  // -- generated final state:
  if ( splitFactor == 1 ) return processFinalState;

  // in Rayl there are no secondaries
  if ( processFinalState->GetNumberOfSecondaries() != 0  ) return processFinalState;

  // -- Now start the biasing:
  // -- We called the Rayl. process above. Its concrete particle change is indeed
  // -- a "G4ParticleChangeForGamma" object. We cast this particle change to access
  // -- methods of the concrete G4ParticleChangeForGamma type:

  G4ParticleChangeForGamma* actualParticleChange =
    ( G4ParticleChangeForGamma* ) processFinalState ;

  fParticleChange.Initialize(*track);

  //*** -- Store first gamma final state:

  //change main photons weight
  G4double gammaWeight = actualParticleChange->GetWeight() / splitFactor;

  G4bool out = outOfRing (track->GetPosition(), actualParticleChange->GetProposedMomentumDirection(), parameters.MyparamsGeometry.detectorY ,
 				  parameters.MyparamsGeometry.detectorY*(-1), parameters.MyparamsGeometry.radius);
  if (!out) // the photon is aimed at the detector
  {
	  fParticleChange.ProposeWeight(gammaWeight);
	  //G4cout << "gammaWeight " << gammaWeight << G4endl;
	  fParticleChange.ProposeTrackStatus( actualParticleChange->GetTrackStatus() );
	  fParticleChange.SetProposedKineticEnergy( actualParticleChange->GetProposedKineticEnergy() );
	  fParticleChange.ProposeMomentumDirection( actualParticleChange->GetProposedMomentumDirection() );
	  fParticleChange.ProposePolarization( actualParticleChange->GetProposedPolarization() );
	  AuxTrackData->fFSState = FSState::toBeFreeFlight;
	  AuxTrackData->SetSecondary();
	  track->SetAuxiliaryTrackInformation(fFSModelID, AuxTrackData);
  }
  else // main photon is heading out - we need to kill him but not his secondaries - after playin RR
  {
	 //play Russian Roullete
	 G4bool survive = RR(1.0/G4double(fSplittingFactorNs));
	 if (survive)
	 {
	   //correct weigth
	   G4double gammaWeightTemp = gammaWeight*(fSplittingFactorNs);
	   AuxTrackData->fFSState = FSState::start;
	   AuxTrackData->SetSecondary();
	   track->SetAuxiliaryTrackInformation(fFSModelID, AuxTrackData);
	   fParticleChange.ProposeWeight(gammaWeightTemp);
	   //G4cout << "gammaWeight " << gammaWeight << G4endl;
	   fParticleChange.ProposeTrackStatus( actualParticleChange->GetTrackStatus() );
	   fParticleChange.SetProposedKineticEnergy( actualParticleChange->GetProposedKineticEnergy() );
	   fParticleChange.ProposeMomentumDirection( actualParticleChange->GetProposedMomentumDirection() );
	   fParticleChange.ProposePolarization( actualParticleChange->GetProposedPolarization() );
	 }
	 else //kill by setting 0 kinetic energy
	 {
	   fParticleChange.ProposeWeight(0.0);
	   //fParticleChange.ProposeTrackStatus( actualParticleChange->GetTrackStatus() );
	   fParticleChange.ProposeTrackStatus( fStopAndKill );
	   fParticleChange.SetProposedKineticEnergy( 0.0 );
	   //THIS IS DONE FOR CONSISTENCY
	   AuxTrackData->fFSState = FSState::start;
	   AuxTrackData->SetSecondary();
	   track->SetAuxiliaryTrackInformation(fFSModelID, AuxTrackData);
     }
  }

  // ***-- inform we will have (fSplittingFactor-1) gamma's + 1 electron:
  fParticleChange.SetNumberOfSecondaries( splitFactor - 1 );

  // -- inform we take care of secondaries weight (otherwise these
  // -- secondaries are by default given the primary weight).
  fParticleChange.SetSecondaryWeightByProcess(true);



   // -- and clean-up the Rayl. process particle change:
   actualParticleChange->Clear();


  // ***-- now start the fSplittingFactor-1 calls to the Rayl. process to store each
  // -- related gamma:
  G4int nCalls = 1;
  G4Track* gammaTrack;
  while ( nCalls < splitFactor )
  {
	  //sample again
	  processFinalState = callingProcess->GetWrappedProcess()->PostStepDoIt(*track, *step);
	  actualParticleChange = ( G4ParticleChangeForGamma* ) processFinalState ;

      if ( processFinalState->GetNumberOfSecondaries() == 0 )
        {
    	  //TODO: is this OK??? Maybe i need to change also the track information??
    	  gammaTrack = new G4Track( *track );

    	  //TODO: note that the trackInformation did not change! all "original" information is kept, will this affect my scoring system???
    	  //tracking information copy is dealt with in post tracking
    	  //TODO: is the track id being set automatically as a child?
    	  //maybe other values of the tracking information needs to be changed!
//    	  G4VUserTrackInformation* info = track->GetUserInformation();


          gammaTrack->SetWeight( gammaWeight );
          gammaTrack->SetKineticEnergy(actualParticleChange->GetProposedKineticEnergy());
		  gammaTrack->SetMomentumDirection(actualParticleChange->GetProposedMomentumDirection());
		  gammaTrack->SetTrackStatus(actualParticleChange->GetTrackStatus());
		  gammaTrack->SetPolarization(actualParticleChange->GetProposedPolarization());

		  //check if directed towards to detector
		  out = outOfRing (gammaTrack->GetPosition(), actualParticleChange->GetProposedMomentumDirection(), parameters.MyparamsGeometry.detectorY ,
				  parameters.MyparamsGeometry.detectorY*(-1), parameters.MyparamsGeometry.radius);
		  if (!out) //directed at the detector
		  {
			  B1BOptrFSTrackData* SecondaryAuxTrackData = new B1BOptrFSTrackData(AuxTrackData->GetOptr());
			  SecondaryAuxTrackData->fFSState = FSState::toBeFreeFlight;
			  SecondaryAuxTrackData->SetSecondary();
			  gammaTrack->SetAuxiliaryTrackInformation(fFSModelID, SecondaryAuxTrackData);
//			  fsplitTracksVector.push_back(gammaTrack);
			  fParticleChange.G4VParticleChange::AddSecondary( gammaTrack );
          }
		  else//not direct to the detector
	      {
			 //play Russian Roullete
			 G4bool survive = RR(1.0/G4double(fSplittingFactorNs));
			 if (survive)
			 {
			   //correct weigth
			   G4double gammaWeightTemp = gammaWeight*(fSplittingFactorNs);
		       gammaTrack->SetWeight( gammaWeightTemp );
			   B1BOptrFSTrackData* SecondaryAuxTrackData = new B1BOptrFSTrackData(AuxTrackData->GetOptr());
			   SecondaryAuxTrackData->fFSState = FSState::start;
			   SecondaryAuxTrackData->SetSecondary();
			   gammaTrack->SetAuxiliaryTrackInformation(fFSModelID, SecondaryAuxTrackData);
//			   fsplitTracksVector.push_back(gammaTrack);
			   fParticleChange.G4VParticleChange::AddSecondary( gammaTrack );

			 }
			 else //just don't add to secondaries list
			 {
			   //kill photon
			 }
	      }
	  }
      // -- very rare special case: we ignore for now.
      //TODO:what is this???
      else if (  processFinalState->GetNumberOfSecondaries() > 1 )
      {
          for ( G4int i = 0 ; i < processFinalState->GetNumberOfSecondaries() ; i++)
            delete processFinalState->GetSecondary(i);
      }

      nCalls++;
      actualParticleChange->Clear();
  }

  // -- we are done:
  return &fParticleChange;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
