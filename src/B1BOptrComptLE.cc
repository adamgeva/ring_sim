
//
#include "B1BOptrComptLE.hh"
#include "B1BOptrComptLETrackData.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4PhysicsModelCatalog.hh"

#include "G4BOptnForceCommonTruncatedExp.hh"
#include "G4ILawCommonTruncatedExp.hh"
#include "G4BOptnForceFreeFlight.hh"
#include "B1BOptnComptSplitting.hh"

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
	fBiasOnlyOnce(true)
{
  params parameters;
  fComptSplittingOperation = new B1BOptnComptSplitting("ComptSplittingOperation");
  fSplittingFactor = parameters.Bias.ComptSplittingFactor;
  fBiasPrimaryOnly = parameters.Bias.BiasPrimaryOnly;
  fBiasOnlyOnce = parameters.Bias.BiasOnlyOnce;
	//fSharedForceInteractionOperation = new G4BOptnForceCommonTruncatedExp("SharedForceInteraction");
  fComptSplittingOperation                = new B1BOptnComptSplitting("ComptSplitting");
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
	fBiasOnlyOnce(true)
{
  params parameters;
  fComptSplittingOperation = new B1BOptnComptSplitting("ComptSplittingOperation");
  fSplittingFactor = parameters.Bias.ComptSplittingFactor;
  fBiasPrimaryOnly = parameters.Bias.BiasPrimaryOnly;
  fBiasOnlyOnce = parameters.Bias.BiasOnlyOnce;
  //fSharedForceInteractionOperation = new G4BOptnForceCommonTruncatedExp("SharedForceInteraction");
  //fCloningOperation                = new G4BOptnCloning("Cloning");
  fComptSplittingOperation         = new B1BOptnComptSplitting("ComptSplitting");
  fParticleToBias                  = particle;
}


B1BOptrComptLE::~B1BOptrComptLE()
{
  for ( std::map< const G4BiasingProcessInterface*, G4BOptnForceFreeFlight* >::iterator it = fFreeFlightOperations.begin() ;
	it != fFreeFlightOperations.end() ;
	it++ ) delete (*it).second;
  //delete fSharedForceInteractionOperation;
  delete fComptSplittingOperation;
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

}


G4VBiasingOperation* B1BOptrComptLE::ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess)
{
  // -- does nothing if particle is not of requested type:
  if ( track->GetDefinition() != fParticleToBias ) return 0;
  G4cout << "ProposeOccurenceBiasingOperation " <<track->GetDefinition()->GetParticleName() <<G4endl;
  G4cout << "ProposeOccurenceBiasingOperation : Calling Process: " << callingProcess->GetProcessName() << G4endl;
  // -- trying to get auxiliary track data...
  if ( fCurrentTrackData == nullptr )
    {
      // ... and if the track has no aux. track data, it means its biasing is not started yet (note that splitting is to happen first):
      fCurrentTrackData = (B1BOptrComptLETrackData*)(track->GetAuxiliaryTrackInformation(flocalEstimationModelID));

      if ( fCurrentTrackData == nullptr ) {
    	  G4cout << "ProposeOccurenceBiasingOperation : No Track DATA" <<G4endl;
    	  return nullptr;
      } else {
    	  G4cout << "ProposeOccurenceBiasingOperation " << G4endl;
    	  fCurrentTrackData->Print();
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
	  G4cout << "ProposeOccurenceBiasingOperation : forceFlight" <<G4endl;
      G4BOptnForceFreeFlight* operation =  fFreeFlightOperations[callingProcess];
      if ( callingProcess->GetWrappedProcess()->GetCurrentInteractionLength() < DBL_MAX/10. )
	  {
		  // -- the initial track weight will be restored only by the first DoIt free flight:
		  operation->ResetInitialTrackWeight(fInitialTrackWeight);
		  G4cout << "ProposeOccurenceBiasingOperation : returning operation free flight" <<G4endl;;
		  return operation;
	  }
      else
	  {
    	  return nullptr;
	  }
  }


//  // -- Send force interaction operation to the callingProcess:
//  // ----------------------------------------------------------
//  // -- at this level, a copy of the track entering the volume was
//  // -- generated (borned) earlier. This copy will make the forced
//  // -- interaction in the volume.
//  if ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeForced )
//    {
//      // -- Remember if this calling process is the first of the physics wrapper in
//      // -- the PostStepGPIL loop (using default argument of method below):
//      G4bool isFirstPhysGPIL = callingProcess-> GetIsFirstPostStepGPILInterface();
//
//      // -- [*first process*] Initialize or update force interaction operation:
//      if ( isFirstPhysGPIL )
//	{
//	  // -- first step of cloned track, initialize the forced interaction operation:
//	  if ( track->GetCurrentStepNumber() == 1 ) fSharedForceInteractionOperation->Initialize( track );
//	  else
//	    {
//	      if ( fSharedForceInteractionOperation->GetInitialMomentum() != track->GetMomentum() )
//		{
//		  // -- means that some other physics process, not under control of the forced interaction operation,
//		  // -- has occured, need to re-initialize the operation as distance to boundary has changed.
//		  // -- [ Note the re-initialization is only possible for a Markovian law. ]
//		  fSharedForceInteractionOperation->Initialize( track );
//		}
//	      else
//		{
//		  // -- means that some other non-physics process (biasing or not, like step limit), has occured,
//		  // -- but track conserves its momentum direction, only need to reduced the maximum distance for
//		  // -- forced interaction.
//		  // -- [ Note the update is only possible for a Markovian law. ]
//		  fSharedForceInteractionOperation->UpdateForStep( track->GetStep() );
//		}
//	    }
//	}
//
//      // -- [*all processes*] Sanity check : it may happen in limit cases that distance to
//      // -- out is zero, weight would be infinite in this case: abort forced interaction
//      // -- and abandon biasing.
//      if ( fSharedForceInteractionOperation->GetMaximumDistance() < DBL_MIN )
//	{
//	  fCurrentTrackData->Reset();
//	  return 0;
//	}
//
//      // -- [* first process*] collect cross-sections and sample force law to determine interaction length
//      // -- and winning process:
//      if ( isFirstPhysGPIL )
//	{
//	  // -- collect cross-sections:
//	  // -- ( Remember that the first of the G4BiasingProcessInterface triggers the update
//	  // --   of these cross-sections )
//	  const G4BiasingProcessSharedData* sharedData = callingProcess->GetSharedData();
//	  for ( size_t i = 0 ; i < (sharedData->GetPhysicsBiasingProcessInterfaces()).size() ; i++ )
//	    {
//	      const G4BiasingProcessInterface* wrapperProcess = ( sharedData->GetPhysicsBiasingProcessInterfaces() )[i];
//	      G4double interactionLength = wrapperProcess->GetWrappedProcess()->GetCurrentInteractionLength();
//	      // -- keep only well defined cross-sections, other processes are ignored. These are not pathological cases
//	      // -- but cases where a threhold effect par example (eg pair creation) may be at play. (**!**)
//	      if ( interactionLength < DBL_MAX/10. )
//		fSharedForceInteractionOperation->AddCrossSection( wrapperProcess->GetWrappedProcess(),  1.0/interactionLength );
//	    }
//	  // -- sample the shared law (interaction length, and winning process):
//	  if ( fSharedForceInteractionOperation->GetNumberOfSharing() > 0 ) fSharedForceInteractionOperation->Sample();
//	}
//
//      // -- [*all processes*] Send operation for processes with well defined XS (see "**!**" above):
//      G4VBiasingOperation* operationToReturn = nullptr;
//      if ( callingProcess->GetWrappedProcess()->GetCurrentInteractionLength() < DBL_MAX/10. ) operationToReturn = fSharedForceInteractionOperation;
//      return operationToReturn;
//
//
//    } // -- end of "if ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeForced )"


  // -- other cases here: particle appearing in the volume by some
  // -- previous interaction : we decide to not bias these.
  return nullptr;

}


//G4VBiasingOperation* B1BOptrComptLE::ProposeNonPhysicsBiasingOperation(const G4Track* track,
//									      const G4BiasingProcessInterface* /* callingProcess */)
//{
////  if ( track->GetDefinition() != fParticleToBias ) return nullptr;
////
////  // -- Apply biasing scheme only to tracks entering the volume.
////  // -- A "cloning" is done:
////  // --  - the primary will be forced flight under a zero weight up to volume exit,
////  // --    where the weight will be restored with proper weight for free flight
////  // --  - the clone will be forced to interact in the volume.
////  if ( track->GetStep()->GetPreStepPoint()->GetStepStatus() == fGeomBoundary ) // -- §§§ extent to case of a track shoot on the boundary ?
////    {
////      // -- check that track is free of undergoing biasing scheme ( no biasing data, or no active active )
////      // -- Get possible track data:
////      fCurrentTrackData = (B1BOptrComptLETrackData*)(track->GetAuxiliaryTrackInformation(flocalEstimationModelID));
////      if ( fCurrentTrackData != nullptr )
////	{
////	  if ( fCurrentTrackData->IsFreeFromBiasing() )
////	    {
////	      // -- takes "ownership" (some track data created before, left free, reuse it):
////	      fCurrentTrackData->flocalEstimationOperator = this ;
////	    }
////	  else
////	    {
////	      // §§§ Would something be really wrong in this case ? Could this be that a process made a zero step ?
////	    }
////	}
////      else
////	{
////	  fCurrentTrackData = new B1BOptrComptLETrackData( this );
////	  track->SetAuxiliaryTrackInformation(flocalEstimationModelID, fCurrentTrackData);
////	}
////      fCurrentTrackData->flocalEstimationState = localEstimationState::toBeCloned;
////      fInitialTrackWeight = track->GetWeight();
////      fCloningOperation->SetCloneWeights(0.0, fInitialTrackWeight);
////      return fCloningOperation;
////    }
////
////  // --
//  return nullptr;
//}


G4VBiasingOperation* B1BOptrComptLE::ProposeFinalStateBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess)
{
  // -- Propose at final state generation the same operation which was proposed at GPIL level,
  // -- (which is either the force free flight or the force interaction operation).
  // -- count on the process interface to collect this operation:
  G4cout << "ProposeFinalStateBiasingOperation : Process name = " << callingProcess->GetProcessName()<<G4endl;


  fCurrentTrackData = (B1BOptrComptLETrackData*)(track->GetAuxiliaryTrackInformation(flocalEstimationModelID));
       if ( fCurrentTrackData != nullptr )
       {
    	   if ( fCurrentTrackData->IsFreeFromBiasing() )
    	   {
			  // -- takes "ownership" (some track data created before, left free, reuse it):
			  fCurrentTrackData->flocalEstimationOperator = this ;
    	   }
    	   else
    	   {
    		   if (fCurrentTrackData->flocalEstimationState == localEstimationState::toBeFreeFlight){
    			   return callingProcess->GetCurrentOccurenceBiasingOperation();

    		   }
    		   // §§§ Would something be really wrong in this case ? Could this be that a process made a zero step ?
    	   }
       }
       else
       {
		  fCurrentTrackData = new B1BOptrComptLETrackData( this );
		  track->SetAuxiliaryTrackInformation(flocalEstimationModelID, fCurrentTrackData);
       }

  if (callingProcess->GetProcessName() != "biasWrapper(compt)") return 0;
  // -- Check if biasing of primary particle only is requested. If so, and
    // -- if particle is not a primary one, don't ask for biasing:
  if ( fBiasPrimaryOnly && ( track->GetParentID() !=0 ) ) return 0;
	// -- Check if brem. splitting should be applied only once to the track,
	// -- and if so, and if brem. splitting already occured, don't ask for biasing:
  if ( fBiasOnlyOnce    && ( fNInteractions > 0 )        ) return 0;

  // -- Count the number of times the brem. splitting is applied:
  fNInteractions++;
  // -- Return the brem. splitting operation:
  G4cout << "ProposeFinalStateBiasingOperation : returning split operation " <<G4endl;
  fInitialTrackWeight = track->GetWeight();
  fCurrentTrackData->flocalEstimationState = localEstimationState::toBeSplit;
  return fComptSplittingOperation;
  //return callingProcess->GetCurrentOccurenceBiasingOperation();
}


void B1BOptrComptLE::StartTracking( const G4Track* track )
{
  fCurrentTrack     = track;
  //TODO: removed because the aux information will be set in the splitting operation, we dont want to reset it here
  fCurrentTrackData = nullptr;
  //fCurrentTrackData = (B1BOptrComptLETrackData*)(track->GetAuxiliaryTrackInformation(flocalEstimationModelID));
  // -- reset the number of times the compton was applied:
//  if (fCurrentTrackData!=nullptr)
//	{
//	  G4cout << "StartTracking :" << G4endl;
//	  fCurrentTrackData->Print();
//    }
//  else
//    {
//	  G4cout << "StartTracking: no trackdata" << G4endl;
//    }
  fNInteractions = 0;
}


void B1BOptrComptLE::EndTracking()
{
  // -- check for consistency, operator should have cleaned the track:
  if ( fCurrentTrackData != nullptr )
    {
      if ( !fCurrentTrackData->IsFreeFromBiasing() )
	{
	  if ( (fCurrentTrack->GetTrackStatus() == fStopAndKill) || (fCurrentTrack->GetTrackStatus() == fKillTrackAndSecondaries) )
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
					      G4VBiasingOperation*             operationApplied,
					      const G4VParticleChange*                          )
{

	//**************printing info***********
	G4cout << "B1BOptrComptLE::OperationApplied 1" <<G4endl;
	G4cout << "calling process: " <<	callingProcess->GetProcessName()  << G4endl;
	//G4cout << "Operation applied : " << operationApplied->GetName() << G4endl;

	//**************************************
  if ( fCurrentTrackData == nullptr )
    {
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

  if  ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeSplit )
    {
      fCurrentTrackData->flocalEstimationState = localEstimationState::free;
      for(G4int i=0;i<fComptSplittingOperation->GetNumOfTracksCopied();i++){
    	  auto cloneData = new B1BOptrComptLETrackData( this );
    	  cloneData->flocalEstimationState = localEstimationState::toBeFreeFlight;
    	  //TODO: maybe I should pop the track from the vector to allow another splitting in the future
    	  fComptSplittingOperation->GetSplitTrack(i)->SetAuxiliaryTrackInformation(flocalEstimationModelID, cloneData);
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


void  B1BOptrComptLE::OperationApplied( const G4BiasingProcessInterface*        callingProcess, G4BiasingAppliedCase                  BAC,
					       G4VBiasingOperation*         /*occurenceOperationApplied*/, G4double             /*weightForOccurenceInteraction*/,
					       G4VBiasingOperation*            finalStateOperationApplied, const G4VParticleChange*    /*particleChangeProduced*/ )
{
	//**************printing info***********
	G4cout << "B1BOptrComptLE::OperationApplied 2 " << G4endl;
	G4cout << "calling process: " << callingProcess->GetProcessName()  << G4endl;
	//G4cout << " Operation applied : " << finalStateOperationApplied->GetName() << G4endl;

	//**************************************

//  if ( fCurrentTrackData->flocalEstimationState == localEstimationState::toBeForced )
//    {
//      if ( finalStateOperationApplied != fSharedForceInteractionOperation )
//	{
//	  G4ExceptionDescription ed;
//	  ed << " Internal inconsistency : please submit bug report. " << G4endl;
//	  G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
//		      "BIAS.GEN.20.5",
//		      JustWarning,
//		      ed);
//	}
//      if ( fSharedForceInteractionOperation->GetInteractionOccured() ) fCurrentTrackData->Reset(); // -- off biasing for this track
//    }
//  else
//    {
//      G4ExceptionDescription ed;
//      ed << " Internal inconsistency : please submit bug report. " << G4endl;
//      G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
//		  "BIAS.GEN.20.6",
//		  JustWarning,
//		  ed);
//    }

}

