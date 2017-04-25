/*
 * B1EnergyDeposit.hh
 *
 *  Created on: Apr 22, 2017
 *      Author: adamgeva
 */
#include "G4PSEnergyDeposit.hh"
#ifndef B1ENERGYDEPOSIT_HH_
#define B1ENERGYDEPOSIT_HH_


class B1EnergyDeposit : public G4PSEnergyDeposit
{
  public:
	B1EnergyDeposit(G4String name);
    virtual ~B1EnergyDeposit();

    virtual G4int GetIndex(G4Step* step);

  protected: // with description
    virtual G4bool ProcessHits(G4Step*,G4TouchableHistory*);

  private:
      //G4int HCID;
      //G4THitsMap<G4double>* EvtMap;
 };



#endif /* B1ENERGYDEPOSIT_HH_ */
