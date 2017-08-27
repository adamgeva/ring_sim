
#include "forcedFlightOperator.hh"


forcedFlightOperator::forcedFlightOperator() : G4VBiasingOperator("forcedFlightOperator")
{
	fForceFreeFlightInteractionLaw = new G4BOptnForceFreeFlight("forcedFlight");
}
forcedFlightOperator::~forcedFlightOperator()
{
	delete fForceFreeFlightInteractionLaw;
}
G4VBiasingOperation *forcedFlightOperator::ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface*)
{
	fForceFreeFlightInteractionLaw->ResetInitialTrackWeight(track->GetWeight());
	return fForceFreeFlightInteractionLaw;
}




