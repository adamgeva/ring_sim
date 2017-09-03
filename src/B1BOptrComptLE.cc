
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
#include "G4BOptnChangeCrossSection.hh"


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

  for ( std::map< const G4BiasingProcessInterface*, G4BOptnChangeCrossSection* >::iterator it = fChangeCrossSectionOperations.begin() ;
    it != fChangeCrossSectionOperations.end() ;
    it++ ) delete (*it).second;

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
        //free flight operations
        G4String operationName = "FreeFlight-"+wrapperProcess->GetWrappedProcess()->GetProcessName();
	    fFreeFlightOperations[wrapperProcess] = new G4BOptnForceFreeFlight(operationName);
	    //cross section operations
	    operationName = "XSchange-" +wrapperProcess->GetWrappedProcess()->GetProcessName();
	    fChangeCrossSectionOperations[wrapperProcess] = new G4BOptnChangeCrossSection(operationName);
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
//    if ( fCurrentTrackData == nullptr )
//    {
//      return nullptr;
//    }
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
  //this is the case that splitting and free flight has begun:
  if (fCurrentTrackData!=nullptr){
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
  }

  //now for the case that splitting and free flight hasn't begun yet or maybe it has begun but we are dealing with the main photon:

  // ---------------------------------------------------------------------
  // -- select and setup the biasing operation for current callingProcess:
  // ---------------------------------------------------------------------
  // -- Check if the analog cross-section well defined : for example, the conversion
  // -- process for a gamma below e+e- creation threshold has an DBL_MAX interaction
  // -- length. Nothing is done in this case (ie, let analog process to deal with the case)
  G4double analogInteractionLength =
	callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
  if ( analogInteractionLength > DBL_MAX/10. ) return 0;

  // -- Analog cross-section is well-defined:
  G4double analogXS = 1./analogInteractionLength;

  // -- Choose a constant cross-section bias. But at this level, this factor can be made
  // -- direction dependent, like in the exponential transform MCNP case, or it
  // -- can be chosen differently, depending on the process, etc.
  G4double XStransformation;
  //G4cout << "process name" << callingProcess->GetWrappedProcess()->GetProcessName() << G4endl;
  //XStransformation = 0.01 ;
  if (callingProcess->GetWrappedProcess()->GetProcessName()=="phot"){
	  //G4cout << "changing cross section for phot" << G4endl;
		XStransformation = 0.01 ;
  } else {
		XStransformation = 1 ;
  }

  // -- fetch the operation associated to this callingProcess:
  G4BOptnChangeCrossSection*   operation = fChangeCrossSectionOperations[callingProcess];
  // -- get the operation that was proposed to the process in the previous step:
  G4VBiasingOperation* previousOperation = callingProcess->GetPreviousOccurenceBiasingOperation();

  // -- now setup the operation to be returned to the process: this
  // -- consists in setting the biased cross-section, and in asking
  // -- the operation to sample its exponential interaction law.
  // -- To do this, to first order, the two lines:
  //        operation->SetBiasedCrossSection( XStransformation * analogXS );
  //        operation->Sample();
  // -- are correct and sufficient.
  // -- But, to avoid having to shoot a random number each time, we sample
  // -- only on the first time the operation is proposed, or if the interaction
  // -- occured. If the interaction did not occur for the process in the previous,
  // -- we update the number of interaction length instead of resampling.
  if ( previousOperation == 0 )
	{
	  operation->SetBiasedCrossSection( XStransformation * analogXS );
	  operation->Sample();
	}
  else
	{
	  if (  previousOperation != operation )
		{
		  // -- should not happen !
		  G4ExceptionDescription ed;
		  ed << " Logic problem in operation handling !" << G4endl;
		  G4Exception("B1BOptrChangeCrossSection::ProposeOccurenceBiasingOperation(...)",
					  "exGB01.02",
					  JustWarning,
					  ed);
		  return 0;
		}
	  if ( operation->GetInteractionOccured() )
		{
		  operation->SetBiasedCrossSection( XStransformation * analogXS );
		  operation->Sample();
		}
	  else
		{
		  // -- update the 'interaction length' and underneath 'number of interaction lengths'
		  // -- for past step  (this takes into accout the previous step cross-section value)
		  operation->UpdateForStep( callingProcess->GetPreviousStepSize() );
		  // -- update the cross-section value:
		  operation->SetBiasedCrossSection( XStransformation * analogXS );
		  // -- forces recomputation of the 'interaction length' taking into account above
		  // -- new cross-section value [tricky : to be improved]
		  operation->UpdateForStep( 0.0 );
		}
	}
  return operation;
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


void B1BOptrComptLE::OperationApplied( const G4BiasingProcessInterface*   callingProcess,
					      G4BiasingAppliedCase                          BAC,
					      G4VBiasingOperation*            operationApplied,
					      const G4VParticleChange*                          )
{

	//todo: code better!
  G4BOptnChangeCrossSection* operation = fChangeCrossSectionOperations[callingProcess];
  if ( operation ==  operationApplied ){
	  operation->SetInteractionOccured();
	  return;
  }


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

