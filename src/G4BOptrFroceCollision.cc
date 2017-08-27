//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
#include "G4BOptrForceCollision.hh"
#include "G4BOptrForceCollisionTrackData.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4PhysicsModelCatalog.hh"

#include "G4BOptnForceCommonTruncatedExp.hh"
#include "G4ILawCommonTruncatedExp.hh"
#include "G4BOptnForceFreeFlight.hh"
#include "G4BOptnCloning.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"

#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

#include "G4SystemOfUnits.hh"

// -- §§ consider calling other constructor, thanks to C++11
G4BOptrForceCollision::G4BOptrForceCollision(G4String particleName, G4String name)
  : G4VBiasingOperator(name),
    fForceCollisionModelID(-1),
    fCurrentTrack(nullptr),
    fCurrentTrackData(nullptr),
    fInitialTrackWeight(-1.0),
    fSetup(true)
{
  fSharedForceInteractionOperation = new G4BOptnForceCommonTruncatedExp("SharedForceInteraction");
  fCloningOperation                = new G4BOptnCloning("Cloning");
  fParticleToBias = G4ParticleTable::GetParticleTable()->FindParticle(particleName);

  if ( fParticleToBias == 0 )
    {
      G4ExceptionDescription ed;
      ed << " Particle `" << particleName << "' not found !" << G4endl;
      G4Exception(" G4BOptrForceCollision::G4BOptrForceCollision(...)",
		  "BIAS.GEN.07",
		  JustWarning,
		  ed);
    }
}


G4BOptrForceCollision::G4BOptrForceCollision(const G4ParticleDefinition* particle, G4String name)
  : G4VBiasingOperator(name),
    fForceCollisionModelID(-1),
    fCurrentTrack(nullptr),
    fCurrentTrackData(nullptr),
    fInitialTrackWeight(-1.0),
    fSetup(true)
{
  fSharedForceInteractionOperation = new G4BOptnForceCommonTruncatedExp("SharedForceInteraction");
  fCloningOperation                = new G4BOptnCloning("Cloning");
  fParticleToBias                  = particle;
}


G4BOptrForceCollision::~G4BOptrForceCollision()
{
  for ( std::map< const G4BiasingProcessInterface*, G4BOptnForceFreeFlight* >::iterator it = fFreeFlightOperations.begin() ;
	it != fFreeFlightOperations.end() ;
	it++ ) delete (*it).second;
  delete fSharedForceInteractionOperation;
  delete fCloningOperation;
}


void G4BOptrForceCollision::Configure()
{
  // -- Create ID for force collision:
  fForceCollisionModelID = G4PhysicsModelCatalog::Register("GenBiasForceCollision");

  // -- build free flight operations:
  ConfigureForWorker();
}


void G4BOptrForceCollision::ConfigureForWorker()
{
  // -- start by remembering processes under biasing, and create needed biasing operations:
  if ( fSetup )
    {
      // -- get back ID created in master thread in case of MT (or reget just created ID in sequential mode):
      fForceCollisionModelID = G4PhysicsModelCatalog::Register("GenBiasForceCollision");

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


void G4BOptrForceCollision::StartRun()
{
}


G4VBiasingOperation* G4BOptrForceCollision::ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess)
{
  // -- does nothing if particle is not of requested type:
  if ( track->GetDefinition() != fParticleToBias ) return 0;


  // -- Send force free flight to the callingProcess:
  // ------------------------------------------------
  // -- The track has been cloned in the previous step, it has now to be
  // -- forced for a free flight.
  // -- This track will fly with 0.0 weight during its forced flight:
  // -- it is to forbid the double counting with the force interaction track.
  // -- Its weight is restored at the end of its free flight, this weight
  // -- being its initial weight * the weight for the free flight travel,
  // -- this last one being per process. The initial weight is common, and is
  // -- arbitrary asked to the first operation to take care of it.

      G4BOptnForceFreeFlight* operation =  fFreeFlightOperations[callingProcess];
      if ( callingProcess->GetWrappedProcess()->GetCurrentInteractionLength() < DBL_MAX/10. )
	{
	  // -- the initial track weight will be restored only by the first DoIt free flight:
	  //operation->ResetInitialTrackWeight(fInitialTrackWeight);
    	  operation->ResetInitialTrackWeight(1.0);
	  return operation;
	}
      else
	{
	  return nullptr;
	}


  return 0;

}


G4VBiasingOperation* G4BOptrForceCollision::ProposeNonPhysicsBiasingOperation(const G4Track* track,
									      const G4BiasingProcessInterface* /* callingProcess */)
{
	 if ( track->GetDefinition() != fParticleToBias ) return nullptr;

	  // -- Apply biasing scheme only to tracks entering the volume.
	  // -- A "cloning" is done:
	  // --  - the primary will be forced flight under a zero weight up to volume exit,
	  // --    where the weight will be restored with proper weight for free flight
	  // --  - the clone will be forced to interact in the volume.
	  if ( track->GetStep()->GetPreStepPoint()->GetStepStatus() == fGeomBoundary ) // -- §§§ extent to case of a track shoot on the boundary ?
	    {
	      fInitialTrackWeight = track->GetWeight();
	      //fCloningOperation->SetCloneWeights(0.0, fInitialTrackWeight);
	      return nullptr;
	    }

	  // --
	  return nullptr;

}


G4VBiasingOperation* G4BOptrForceCollision::ProposeFinalStateBiasingOperation(const G4Track*, const G4BiasingProcessInterface* callingProcess)
{
  // -- Propose at final state generation the same operation which was proposed at GPIL level,
  // -- (which is either the force free flight or the force interaction operation).
  // -- count on the process interface to collect this operation:
  return callingProcess->GetCurrentOccurenceBiasingOperation();
  //return nullptr;
}


void G4BOptrForceCollision::StartTracking( const G4Track* track )
{
  fCurrentTrack     = track;
  //fCurrentTrackData = nullptr;
}


void G4BOptrForceCollision::EndTracking()
{

}


void G4BOptrForceCollision::OperationApplied( const G4BiasingProcessInterface*   callingProcess,
					      G4BiasingAppliedCase                          BAC,
					      G4VBiasingOperation*             operationApplied,
					      const G4VParticleChange*                          )
{

//  if ( fCurrentTrackData == nullptr )
//    {
//      if ( BAC != BAC_None )
//	{
//	  G4ExceptionDescription ed;
//	  ed << " Internal inconsistency : please submit bug report. " << G4endl;
//	  G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
//		      "BIAS.GEN.20.1",
//		      JustWarning,
//		      ed);
//	}
//      return;
//    }
//
//  if      ( fCurrentTrackData->fForceCollisionState == ForceCollisionState::toBeCloned )
//    {
//      fCurrentTrackData->fForceCollisionState = ForceCollisionState::toBeFreeFlight;
//      auto cloneData                  = new G4BOptrForceCollisionTrackData( this );
//      cloneData->fForceCollisionState = ForceCollisionState::toBeForced;
//      fCloningOperation->GetCloneTrack()->SetAuxiliaryTrackInformation(fForceCollisionModelID, cloneData);
//    }
//  else if ( fCurrentTrackData->fForceCollisionState == ForceCollisionState::toBeFreeFlight )
//    {
//      if ( fFreeFlightOperations[callingProcess]->OperationComplete() ) fCurrentTrackData->Reset(); // -- off biasing for this track
//    }
//  else if ( fCurrentTrackData->fForceCollisionState == ForceCollisionState::toBeForced )
//    {
//      if ( operationApplied != fSharedForceInteractionOperation )
//	{
//	  G4ExceptionDescription ed;
//	  ed << " Internal inconsistency : please submit bug report. " << G4endl;
//	  G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
//		      "BIAS.GEN.20.2",
//		      JustWarning,
//		      ed);
//	}
//      if ( fSharedForceInteractionOperation->GetInteractionOccured() )
//	{
//	  if ( operationApplied != fSharedForceInteractionOperation )
//	    {
//	      G4ExceptionDescription ed;
//	      ed << " Internal inconsistency : please submit bug report. " << G4endl;
//	      G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
//			  "BIAS.GEN.20.3",
//			  JustWarning,
//			  ed);
//	    }
//	}
//    }
//  else
//    {
//      if ( fCurrentTrackData->fForceCollisionState != ForceCollisionState::free )
//	{
//	  G4ExceptionDescription ed;
//	  ed << " Internal inconsistency : please submit bug report. " << G4endl;
//	  G4Exception(" G4BOptrForceCollision::OperationApplied(...)",
//		      "BIAS.GEN.20.4",
//		      JustWarning,
//		      ed);
//	}
//    }
}


void  G4BOptrForceCollision::OperationApplied( const G4BiasingProcessInterface*        /*callingProcess*/, G4BiasingAppliedCase                  /*biasingCase*/,
					       G4VBiasingOperation*         /*occurenceOperationApplied*/, G4double             /*weightForOccurenceInteraction*/,
					       G4VBiasingOperation*            finalStateOperationApplied, const G4VParticleChange*    /*particleChangeProduced*/ )
{

//  if ( fCurrentTrackData->fForceCollisionState == ForceCollisionState::toBeForced )
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

