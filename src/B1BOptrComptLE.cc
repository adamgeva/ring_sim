
//
#include "B1BOptrComptLE.hh"
#include "B1BOptrComptLETrackData.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4PhysicsModelCatalog.hh"

#include "G4BOptnForceCommonTruncatedExp.hh"
#include "G4ILawCommonTruncatedExp.hh"
#include "G4BOptnForceFreeFlight.hh"
#include "B1BOptnComptSplitting.hh"
#include "B1BOptnRaylSplitting.hh"


#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"

#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

#include "G4SystemOfUnits.hh"
#include "params.hh"

// -- §§ consider calling other constructor, thanks to C++11
B1BOptrComptLE::B1BOptrComptLE(G4String particleName, G4String name)
  : G4VBiasingOperator(name),
    flocalEstimationModelID(-1),
    fCurrentTrack(nullptr),
    fCurrentTrackData(nullptr),
    fInitialTrackWeight(-1.0),
    fSetup(true),
	fSplittingFactor(1),
	fBiasPrimaryOnly(true),
	fBiasOnlyOnce(true),
	fBiasNTimes(10),
	fNInteractions(0)
{
  params parameters;
  fSplittingFactor = parameters.Bias.ComptSplittingFactor;
  fBiasPrimaryOnly = parameters.Bias.BiasPrimaryOnly;
  fBiasOnlyOnce = parameters.Bias.BiasOnlyOnce;
  fBiasNTimes = parameters.Bias.BiasTimes;
	//fSharedForceInteractionOperation = new G4BOptnForceCommonTruncatedExp("SharedForceInteraction");
  fComptSplittingOperation                = new B1BOptnComptSplitting("ComptSplitting");
  fRaylSplittingOperation                = new B1BOptnRaylSplitting("RaylSplitting");
  fParticleToBias = G4ParticleTable::GetParticleTable()->FindParticle(particleName);

  if ( fParticleToBias == 0 )
    {
      G4ExceptionDescription ed;
      ed << " Particle `" << particleName << "' not found !" << G4endl;
      G4Exception(" B1BOptrComptLE::B1BOptrComptLE(...)",
		  "BIAS.GEN.07",
		  JustWarning,
		  ed);
    }
}


B1BOptrComptLE::B1BOptrComptLE(const G4ParticleDefinition* particle, G4String name)
  : G4VBiasingOperator(name),
    flocalEstimationModelID(-1),
    fCurrentTrack(nullptr),
    fCurrentTrackData(nullptr),
    fInitialTrackWeight(-1.0),
    fSetup(true),
	fSplittingFactor(1),
	fBiasPrimaryOnly(true),
	fBiasOnlyOnce(true),
	fBiasNTimes(10),
	fNInteractions(0)
{
  params parameters;
  fSplittingFactor = parameters.Bias.ComptSplittingFactor;
  fBiasPrimaryOnly = parameters.Bias.BiasPrimaryOnly;
  fBiasOnlyOnce = parameters.Bias.BiasOnlyOnce;
  fBiasNTimes = parameters.Bias.BiasTimes;
  //fSharedForceInteractionOperation = new G4BOptnForceCommonTruncatedExp("SharedForceInteraction");
  //fCloningOperation                = new G4BOptnCloning("Cloning");
  fComptSplittingOperation         = new B1BOptnComptSplitting("ComptSplitting");
  fRaylSplittingOperation         = new B1BOptnRaylSplitting("RaylSplitting");
  fParticleToBias                  = particle;
}


B1BOptrComptLE::~B1BOptrComptLE()
{
  for ( std::map< const G4BiasingProcessInterface*, G4BOptnForceFreeFlight* >::iterator it = fFreeFlightOperations.begin() ;
	it != fFreeFlightOperations.end() ;
	it++ ) delete (*it).second;
  //delete fSharedForceInteractionOperation;
  delete fComptSplittingOperation;
  delete fRaylSplittingOperation;
}


void B1BOptrComptLE::Configure()
{
  // -- Create ID for compton local estimation:
  flocalEstimationModelID = G4PhysicsModelCatalog::Register("GenBiaslocalEstimation");
  // -- build free flight operations:
  ConfigureForWorker();
}


void B1BOptrComptLE::ConfigureForWorker()
{
  // -- start by remembering processes under biasing, and create needed biasing operations:
  if ( fSetup )
  {
    // -- get back ID created in master thread in case of MT (or reget just created ID in sequential mode):
    flocalEstimationModelID = G4PhysicsModelCatalog::Register("GenBiaslocalEstimation");

    const G4ProcessManager* processManager = fParticleToBias->GetProcessManager();
    const G4BiasingProcessSharedData* interfaceProcessSharedData = G4BiasingProcessInterface::GetSharedData( processManager );
    if ( interfaceProcessSharedData ) // -- sharedData tested, as is can happen a user attaches an operator
	                                // -- to a volume but without defining BiasingProcessInterface processes.
    {
      for ( size_t i = 0 ; i < (interfaceProcessSharedData->GetPhysicsBiasingProcessInterfaces()).size(); i++ )
	  {
        const G4BiasingProcessInterface* wrapperProcess =
        (interfaceProcessSharedData->GetPhysicsBiasingProcessInterfaces())[i];
        G4String operationName = "FreeFlight-"+wrapperProcess->GetWrappedProcess()->GetProcessName();
	    fFreeFlightOperations[wrapperProcess] = new G4BOptnForceFreeFlight(operationName);
	  }
	}
    fSetup = false;
  }
}


void B1BOptrComptLE::StartRun()
{
  fComptSplittingOperation->SetSplittingFactor ( fSplittingFactor );
  fRaylSplittingOperation->SetSplittingFactor ( fSplittingFactor );
}


G4VBiasingOperation* B1BOptrComptLE::ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess)
{
  // -- does nothing if particle is not of requested type:
  if ( track->GetDefinition() != fParticleToBias ) return 0;
  // -- trying to get auxiliary track data...
  if ( fCurrentTrackData == nullptr )
  {
    // ... and if the track has no aux. track data, it means its biasing is not started yet (note that splitting is to happen first):
    fCurrentTrackData = (B1BOptrComptLETrackData*)(track->GetAuxiliaryTrackInformation(flocalEstimationModelID));
    if ( fCurrentTrackData == nullptr )
    {
      return nullptr;
    }
  }
  // -- Send force free flight to the callingProcess:
  // ------------------------------------------------
  // -- The track has been split in the previous step, it has now to be
  // -- forced for a free flight.
  // -- This track will fly with 0.0 weight during its forced flight: ??? maybe not 0??
  // -- it is to forbid the double counting with the force interaction track. no force interaction!
  // -- Its weight is restored at the end of its free flight, this weight
  // -- being its initial weight * the weight for the free flight travel,
  // -- this last one being per process. The initial weight is common, and is
  // -- arbitrary asked to the first operation to take care of it.
  if ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeFreeFlight)
  {
    G4BOptnForceFreeFlight* operation =  fFreeFlightOperations[callingProcess];
    if ( callingProcess->GetWrappedProcess()->GetCurrentInteractionLength() < DBL_MAX/10. )
	{
	  // -- the initial track weight will be restored only by the first DoIt free flight:
	  operation->ResetInitialTrackWeight(fInitialTrackWeight/fSplittingFactor);
      return operation;
    }
    else
    {
   	  return nullptr;
    }
  }

  // -- other cases here: particle appearing in the volume by some
  // -- previous interaction : we decide to not bias these.
  return 0;

}


G4VBiasingOperation* B1BOptrComptLE::ProposeFinalStateBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess)
{

  //fetch track data
  fCurrentTrackData = (B1BOptrComptLETrackData*)(track->GetAuxiliaryTrackInformation(flocalEstimationModelID));
  if ( fCurrentTrackData != nullptr )
  {
     if ( fCurrentTrackData->IsFreeFromBiasing() )
     {
  	  // -- takes "ownership" (some track data created before, left free, reuse it). for example if there was Rayl before Compt on the main particle
	   fCurrentTrackData->flocalEstimationOperator = this ;
     }
     else
     {
	   //in case the particle is free flying
	   if (fCurrentTrackData->flocalEstimationState == localEstimationState::toBeFreeFlight)
	   {
		 //!!!!
         return callingProcess->GetCurrentOccurenceBiasingOperation();
	   }
     }
  }
  else
  {
    fCurrentTrackData = new B1BOptrComptLETrackData( this );
    track->SetAuxiliaryTrackInformation(flocalEstimationModelID, fCurrentTrackData);
  }


  // -- Check if biasing of primary particle only is requested. If so, and
    // -- if particle is not a primary one, don't ask for biasing:
  if ( fBiasPrimaryOnly && ( track->GetParentID() !=0 ) ) return 0;
	// -- Check if brem. splitting should be applied only once to the track,
	// -- and if so, and if brem. splitting already occured, don't ask for biasing:
  if ( fBiasOnlyOnce    && ( fNInteractions > 0 )        ) return 0;

  if ( fNInteractions > fBiasNTimes ) return 0;

  //check if its time for compton splitting:
  if (callingProcess->GetProcessName() == "biasWrapper(compt)")
  {
	  //send compton splitting operation:
	  // -- Count the number of times the brem. splitting is applied:
	  fNInteractions++;
	  // -- Return the compt. splitting operation:
	  fInitialTrackWeight = track->GetWeight();
	  fCurrentTrackData->flocalEstimationState = localEstimationState::toBeSplitCompt;
	  return fComptSplittingOperation;
  }
  //check if its time for rayl splitting:
  else if (callingProcess->GetProcessName() == "biasWrapper(Rayl)")
  {
	  //send Rayl splitting operation:
	  // -- Count the number of times the brem. splitting is applied:
	  fNInteractions++;
	  // -- Return the compt. splitting operation:
	  fInitialTrackWeight = track->GetWeight();
	  fCurrentTrackData->flocalEstimationState = localEstimationState::toBeSplitRayl;
	  return fRaylSplittingOperation;
  }
  return 0;
}


void B1BOptrComptLE::StartTracking( const G4Track* track )
{
  fCurrentTrack     = track;
  fCurrentTrackData = nullptr;
  //for every new track number of compton interaction=0
  fNInteractions = 0;
}


void B1BOptrComptLE::EndTracking()
{
  // -- check for consistency, operator should have cleaned the track:
  if ( fCurrentTrackData != nullptr )
  {
    if ( !fCurrentTrackData->IsFreeFromBiasing() )
	{
	  if (fCurrentTrackData->flocalEstimationState == localEstimationState::toBeFreeFlight)
	  {
	    return;
	  }
	  else if ( (fCurrentTrack->GetTrackStatus() == fStopAndKill) || (fCurrentTrack->GetTrackStatus() == fKillTrackAndSecondaries) )
	  {
	    G4ExceptionDescription ed;
	    ed << "Current track deleted while under biasing by " << GetName() << ". Will result in inconsistencies.";
	    G4Exception(" B1BOptrComptLE::EndTracking()",
	    "BIAS.GEN.18",
			  JustWarning,
			  ed);
	  }
	}
  }
}


void B1BOptrComptLE::OperationApplied( const G4BiasingProcessInterface*   /*callingProcess*/,
					      G4BiasingAppliedCase                          BAC,
					      G4VBiasingOperation*            /* operationApplied*/,
					      const G4VParticleChange*                          )
{
  //nothing
  if ( fCurrentTrackData == nullptr )
  {
	//impossible that an operation was applied with no track data...
    if ( BAC != BAC_None )
	{
	  G4ExceptionDescription ed;
	  ed << " Internal inconsistency : please submit bug report. " << G4endl;
	  G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
		      "BIAS.GEN.20.1",
		      JustWarning,
		      ed);
	}
    return;
  }
  //tobisplit
  if  ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeSplitCompt )
  {
    fCurrentTrackData->flocalEstimationState = localEstimationState::free;
    G4int numOfCopies = fComptSplittingOperation->GetNumOfTracksCopied();
    for(G4int i=0;i<numOfCopies;i++)
    {

      auto cloneData = new B1BOptrComptLETrackData( this );
      cloneData->flocalEstimationState = localEstimationState::toBeFreeFlight;
      //TODO: maybe I should pop the track from the vector to allow another splitting in the future
   	  fComptSplittingOperation->GetSplitTrack()->SetAuxiliaryTrackInformation(flocalEstimationModelID, cloneData);
    }
  }
  else if  ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeSplitRayl )
    {
      fCurrentTrackData->flocalEstimationState = localEstimationState::free;
      G4int numOfCopies = fRaylSplittingOperation->GetNumOfTracksCopied();
      for(G4int i=0;i<numOfCopies;i++)
      {

        auto cloneData = new B1BOptrComptLETrackData( this );
        cloneData->flocalEstimationState = localEstimationState::toBeFreeFlight;
        //TODO: maybe I should pop the track from the vector to allow another splitting in the future
     	  fRaylSplittingOperation->GetSplitTrack()->SetAuxiliaryTrackInformation(flocalEstimationModelID, cloneData);
      }
    }
  else if ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeFreeFlight )
  {
	  //TODO:how can it work with this line?
      //if ( fFreeFlightOperations[callingProcess]->OperationComplete() ) fCurrentTrackData->Reset(); // -- off biasing for this track
  }
  else
  {
    if ( fCurrentTrackData->flocalEstimationState != localEstimationState::free )
	{
	  G4ExceptionDescription ed;
	  ed << " Internal inconsistency : please submit bug report. " << G4endl;
	  G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
		      "BIAS.GEN.20.4",
		      JustWarning,
		      ed);
	}
  }
}


void B1BOptrComptLE::OperationApplied( const G4BiasingProcessInterface* /*callingProcess*/, G4BiasingAppliedCase /*BAC*/,
			 G4VBiasingOperation* /*occurenceOperationApplied*/, G4double /*weightForOccurenceInteraction*/,
			 G4VBiasingOperation* /*finalStateOperationApplied*/, const G4VParticleChange* /*particleChangeProduced*/ )
{
	  //nothing

}

