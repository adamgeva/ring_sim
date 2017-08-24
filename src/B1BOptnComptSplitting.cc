#include "B1BOptnComptSplitting.hh"
#include "B1BOptrComptLE.hh"
#include <vector>

#include "G4BiasingProcessInterface.hh"
#include "G4ParticleChangeForGamma.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1BOptnComptSplitting::B1BOptnComptSplitting(G4String name)
: G4VBiasingOperation(name),
  fSplittingFactor(1),
  fParticleChange()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1BOptnComptSplitting::~B1BOptnComptSplitting()
{
}

G4Track* B1BOptnComptSplitting::GetSplitTrack(G4int ID)
{
	if (!fsplitTracksVector.empty()) {
		return fsplitTracksVector[ID];
	}
	return nullptr;
}

G4VParticleChange*
B1BOptnComptSplitting::
ApplyFinalStateBiasing( const G4BiasingProcessInterface* callingProcess,
                        const G4Track*                            track,
                        const G4Step*                              step,
                        G4bool&                                         )
{

  // -- Collect compt. process (wrapped process) final state:
  G4VParticleChange* processFinalState =
    callingProcess->GetWrappedProcess()->PostStepDoIt(*track, *step);

  // -- if no splitting requested, let the compton. process to return directly its
  // -- generated final state:
  if ( fSplittingFactor == 1 ) return processFinalState;

  // -- a special case here: the brem. process corrects for cross-section change
  // -- over the step due to energy loss by sometimes "abandoning" the interaction,
  // -- returning an unchanged incoming electron/positron.
  // -- We respect this correction, and if no secondary is produced, its means this
  // -- case is happening:
  //TODO: I think this is relevent for compton process as well. In case the interaction has been cancelled there will not be an electorn as a secondary particle.
  if ( processFinalState->GetNumberOfSecondaries() == 0 )  return processFinalState;

  // -- Now start the biasing:
  // --   - the electron state will be taken as the first one produced by the brem.
  // --     process, hence the one stored in above processFinalState particle change.
  // --     This state will be stored in our fParticleChange object.
  // --   - the photon accompagnying the electron will be stored also this way.
  // --   - we will then do fSplittingFactor - 1 call to the brem. process to collect
  // --     fSplittingFactor - 1 additionnal gammas. All these will be stored in our
  // --     fParticleChange object.

  // -- We called the compton. process above. Its concrete particle change is indeed
  // -- a "G4ParticleChangeForGamma" object. We cast this particle change to access
  // -- methods of the concrete G4ParticleChangeForGamma type:

  G4ParticleChangeForGamma* actualParticleChange =
    ( G4ParticleChangeForGamma* ) processFinalState ;

  fParticleChange.Initialize(*track);

  // -- Store first gamma final state:


  fParticleChange.ProposeTrackStatus( actualParticleChange->GetTrackStatus() );
  fParticleChange.SetProposedKineticEnergy( actualParticleChange->GetProposedKineticEnergy() );
  fParticleChange.ProposeMomentumDirection( actualParticleChange->GetProposedMomentumDirection() );
  fParticleChange.ProposePolarization( actualParticleChange->GetProposedPolarization() );

  //change main photons weight
  G4double gammaWeight = actualParticleChange->GetWeight() / fSplittingFactor;
  fParticleChange.ProposeWeight(gammaWeight);
  //G4cout << "gammaWeight " << gammaWeight << G4endl;

  // -- inform we will have (fSplittingFactor-1) gamma's + 1 electron:
  fParticleChange.SetNumberOfSecondaries( fSplittingFactor );

  // -- inform we take care of secondaries weight (otherwise these
  // -- secondaries are by default given the primary weight).
  fParticleChange.SetSecondaryWeightByProcess(true);


  //store electron final state:
   G4Track* electronTrack = actualParticleChange->GetSecondary(0);
   //TODO: is this necessary?
   electronTrack->SetWeight( electronTrack->GetWeight() );
   //G4cout << "electron Weight " << electronTrack->GetWeight() << G4endl;

   //TODO:check that is correct call to mother method?
   fParticleChange.G4VParticleChange::AddSecondary( electronTrack );
   // -- and clean-up the compt. process particle change:
   actualParticleChange->Clear();


  // -- now start the fSplittingFactor-1 calls to the compt. process to store each
  // -- related gamma:
  G4int nCalls = 1;
  G4Track* gammaTrack;
  while ( nCalls < fSplittingFactor )
    {
	  //sample again
	  processFinalState = callingProcess->GetWrappedProcess()->PostStepDoIt(*track, *step);
	  actualParticleChange = ( G4ParticleChangeForGamma* ) processFinalState ;

      if ( processFinalState->GetNumberOfSecondaries() == 1 )
        {
    	  //TODO: is this OK??? Maybe i need to change also the track information??
    	  gammaTrack = new G4Track( *track );

    	  //TODO: note that the trackInformation did not change! all "original" information is kept, will this affect my scoring system???
    	  //TODO: is the track id being set automatically as a child?
    	  //maybe other values of the tracking information needs to be changed!
//    	  G4VUserTrackInformation* info = track->GetUserInformation();
//    	  B1TrackInformation* theInfo = (B1TrackInformation*)info;
//    	  theInfo->SetForceFlight();
//    	  gammaTrack->SetUserInformation(theInfo);

          gammaTrack->SetWeight( gammaWeight );
          gammaTrack->SetKineticEnergy(actualParticleChange->GetProposedKineticEnergy());
		  gammaTrack->SetMomentumDirection(actualParticleChange->GetProposedMomentumDirection());
		  gammaTrack->SetTrackStatus(actualParticleChange->GetTrackStatus());
		  gammaTrack->SetPolarization(actualParticleChange->GetProposedPolarization());

		  fsplitTracksVector.push_back(gammaTrack);
          fParticleChange.G4VParticleChange::AddSecondary( gammaTrack );
          nCalls++;
        }
      // -- very rare special case: we ignore for now.
      //TODO:what is this???
      else if (  processFinalState->GetNumberOfSecondaries() > 1 )
        {
          for ( G4int i = 0 ; i < processFinalState->GetNumberOfSecondaries() ; i++)
            delete processFinalState->GetSecondary(i);
        }
      actualParticleChange->Clear();
    }

  // -- we are done:
  return &fParticleChange;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
