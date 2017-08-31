
#ifndef B1BOptrComptLETrackData_hh
#define B1BOptrComptLETrackData_hh

class B1BOptrComptLE;
#include "G4VAuxiliaryTrackInformation.hh"

enum class localEstimationState { free, toBeFreeFlight, toBeSplitCompt, toBeSplitRayl };

class B1BOptrComptLETrackData : public G4VAuxiliaryTrackInformation {

friend class B1BOptrComptLE;

public:
  B1BOptrComptLETrackData(  const B1BOptrComptLE* optr );
  ~B1BOptrComptLETrackData();

  // -- from base class:
  void Print() const;

  // -- Get methods:
  G4bool                             IsFreeFromBiasing() const
  { return ( flocalEstimationState == localEstimationState::free);}



  // -- no set methods are provided : sets are made under exclusive control of B1BOptrComptLE objects through friendness.

private:
  const B1BOptrComptLE*            flocalEstimationOperator;
  localEstimationState             flocalEstimationState;

  void Reset()
    {
      flocalEstimationOperator = nullptr;
      flocalEstimationState    = localEstimationState::free;
    }



};

#endif
