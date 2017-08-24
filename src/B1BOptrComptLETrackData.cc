//
#include "B1BOptrComptLETrackData.hh"
#include "B1BOptrComptLE.hh"


B1BOptrComptLETrackData::B1BOptrComptLETrackData( const B1BOptrComptLE* optr )
: G4VAuxiliaryTrackInformation(),
  flocalEstimationOperator( optr )
{
  flocalEstimationState = localEstimationState::free;
}

B1BOptrComptLETrackData::~B1BOptrComptLETrackData()
{
	if ( flocalEstimationState != localEstimationState::free )
	    {
	      G4ExceptionDescription ed;
	      ed << "Track deleted while under G4BOptrForceCollision biasing scheme of operator `";
	      if ( flocalEstimationOperator == nullptr ) ed << "(none)"; else ed << flocalEstimationOperator->GetName();
	      ed <<"'. Will result in inconsistencies.";
	      G4Exception(" B1BOptrComptLETrackData::~B1BOptrComptLETrackData()",
			  "BIAS.GEN.19",
			  JustWarning,
			  ed);
	    }
}

void B1BOptrComptLETrackData::Print() const
{
  G4cout << " B1BOptrComptLETrackData object : " << this << G4endl;
  G4cout << "     Local Estimation operator : "; if ( flocalEstimationOperator == nullptr ) G4cout << "(none)"; else G4cout << flocalEstimationOperator->GetName(); G4cout << G4endl;
  G4cout << "     Local Estimation state    : ";
  switch ( flocalEstimationState )
    {
    case localEstimationState::free :
      G4cout << "free from biasing ";
      break;
    case localEstimationState::toBeSplit :
      G4cout << "to be cloned ";
      break;
    case localEstimationState::toBeFreeFlight :
      G4cout << "to be free flight forced ";
      break;
    default:
      break;
    }
  G4cout << G4endl;
}
