/*
 * forcedFlightOperator.hh
 *
 *  Created on: Aug 26, 2017
 *      Author: adamgeva
 */

#ifndef FORCEDFLIGHTOPERATOR_HH_
#define FORCEDFLIGHTOPERATOR_HH_


#include "G4VBiasingOperator.hh"
#include "G4BOptnForceFreeFlight.hh"
class forcedFlightOperator : public G4VBiasingOperator
{
public:
	forcedFlightOperator();
	virtual ~forcedFlightOperator();
	G4BOptnForceFreeFlight *GetForceFreeFlightLaw() {return fForceFreeFlightInteractionLaw;};
private:
	G4BOptnForceFreeFlight *fForceFreeFlightInteractionLaw;
	// used for forced flight
	virtual G4VBiasingOperation *ProposeOccurenceBiasingOperation(const G4Track*, const G4BiasingProcessInterface* );
	// unused but required pure virtual functions
	virtual G4VBiasingOperation *ProposeNonPhysicsBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess) { return 0; };
        virtual G4VBiasingOperation *ProposeFinalStateBiasingOperation(const G4Track*, const G4BiasingProcessInterface* ) { return 0; };
};



#endif /* FORCEDFLIGHTOPERATOR_HH_ */
