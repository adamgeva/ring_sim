

#ifndef B1BOptnComptSplitting_hh
#define B1BOptnComptSplitting_hh 1

#include "G4VBiasingOperation.hh"
//#include "G4ParticleChange.hh"
#include "G4ParticleChangeForGamma.hh"
#include <vector>

class B1BOptnComptSplitting : public G4VBiasingOperation {
public:
  // -- Constructor :
  B1BOptnComptSplitting(G4String name);
  // -- destructor:
  virtual ~B1BOptnComptSplitting();


public:
  // ----------------------------------------------
  // -- Methods from G4VBiasingOperation interface:
  // ----------------------------------------------
  // -- Unused:
  virtual const G4VBiasingInteractionLaw*
  ProvideOccurenceBiasingInteractionLaw( const G4BiasingProcessInterface*,
                                         G4ForceCondition& )
  { return 0; }

  // --Used:
  virtual G4VParticleChange*   ApplyFinalStateBiasing( const G4BiasingProcessInterface*,
                                                       const G4Track*,
                                                       const G4Step*,
                                                       G4bool&                          );

  // -- Unsued:
  virtual G4double           DistanceToApplyOperation( const G4Track*,
                                                       G4double,
                                                       G4ForceCondition*)
  {return DBL_MAX;}
  virtual G4VParticleChange* GenerateBiasingFinalState( const G4Track*,
                                                        const G4Step*   )
  {return 0;}


public:
  // ----------------------------------------------
  // -- Additional methods, specific to this class:
  // ----------------------------------------------
  // -- Splitting factor:
  void     SetSplittingFactor(G4int splittingFactor)
  { fSplittingFactor = splittingFactor; }
  G4int GetSplittingFactor() const
  { return fSplittingFactor; }

  G4Track* GetSplitTrack(G4int ID);
  G4int GetNumOfTracksCopied()
  { return fsplitTracksVector.size(); }



private:
  G4int            fSplittingFactor;
  //G4ParticleChange fParticleChange;
  G4ParticleChangeForGamma fParticleChange;
  std::vector<G4Track*>  fsplitTracksVector;

};

#endif
