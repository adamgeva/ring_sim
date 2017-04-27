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
	B1EnergyDeposit(G4String name, G4int type);
    virtual ~B1EnergyDeposit();

    virtual G4int GetIndex(G4Step* step);

  protected: // with description
    virtual G4bool ProcessHits(G4Step*,G4TouchableHistory*);

  private:
    G4bool recordInteraction (G4Step* aStep,G4TouchableHistory* touchable, G4int totalNumOfInteractions, G4int i);
      //G4int HCID;
      //G4THitsMap<G4double>* EvtMap;
    //scorer type: 1=no_scatter, 2=include_single_scatter, 3=include_multi_scatter, 4=include_single_scatter_compt, 5=include_single_scatter_Rayl
    G4int fscorerType;
 };



#endif /* B1ENERGYDEPOSIT_HH_ */
