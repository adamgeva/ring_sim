


#ifndef B1BOptrComptLE_hh
#define B1BOptrComptLE_hh 1

#include "G4VBiasingOperator.hh"
#include "G4BOptnChangeCrossSection.hh"
#include "B1BOptnComptSplitting.hh"
#include "B1BOptnRaylSplitting.hh"
class G4BOptnForceFreeFlight;
class G4BOptnForceCommonTruncatedExp;
class G4BOptnCloning;
class G4VProcess;
class G4BiasingProcessInterface;
class G4ParticleDefinition;
#include <vector>
#include <map>
#include "G4ThreeVector.hh"
class B1BOptrComptLETrackData;

class B1BOptrComptLE : public G4VBiasingOperator {
public:
  B1BOptrComptLE(G4String particleToForce,                    G4String name="localEstimation");
  B1BOptrComptLE(const G4ParticleDefinition* particleToForce, G4String name="localEstimation");
  ~B1BOptrComptLE();

private:
  // -- Mandatory from base class :
  virtual G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(const G4Track* , const G4BiasingProcessInterface* ){
	  return 0;
  }
  virtual G4VBiasingOperation*  ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess) final;
  virtual G4VBiasingOperation* ProposeFinalStateBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess) final;
  // -- optional methods from base class:
public:
  virtual void           Configure() final;
  virtual void  ConfigureForWorker() final;
  virtual void            StartRun() final;
  virtual void       StartTracking( const G4Track* track ) final;
  virtual void         ExitBiasing( const G4Track*, const G4BiasingProcessInterface* ) final {};
  virtual void         EndTracking() final;

  // -- operation applied:
  void OperationApplied( const G4BiasingProcessInterface*            callingProcess, G4BiasingAppliedCase                      biasingCase,
			 G4VBiasingOperation*                      operationApplied, const G4VParticleChange*        particleChangeProduced ) final;
  void OperationApplied( const G4BiasingProcessInterface* callingProcess, G4BiasingAppliedCase biasingCase,
  			 G4VBiasingOperation* occurenceOperationApplied, G4double weightForOccurenceInteraction,
  			 G4VBiasingOperation* finalStateOperationApplied, const G4VParticleChange* particleChangeProduced ) final;


private:
  G4int                                                                 flocalEstimationModelID;
  const G4Track*                                                        fCurrentTrack;
  B1BOptrComptLETrackData*                                              fCurrentTrackData;
  std::map< const G4BiasingProcessInterface*, G4BOptnForceFreeFlight* > fFreeFlightOperations;

  // -- List of associations between processes and biasing operations:
  std::map< const G4BiasingProcessInterface*, G4BOptnChangeCrossSection* > fChangeCrossSectionOperations;


  //G4BOptnForceCommonTruncatedExp*                                       fSharedForceInteractionOperation;
  B1BOptnComptSplitting*                                                fComptSplittingOperation;
  B1BOptnRaylSplitting*                                                 fRaylSplittingOperation;
  G4double                                                              fInitialTrackWeight;
  G4bool                                                                fSetup;
  const G4ParticleDefinition*                                           fParticleToBias;

  G4int                          								        fSplittingFactor;
  G4bool                        									    fBiasPrimaryOnly;
  G4bool                         									    fBiasOnlyOnce;
  G4int 																fBiasNTimes;

  G4int                 									            fNInteractions;


};

#endif
