/*
 * B1Run.hh
 *
 *  Created on: Apr 22, 2017
 *      Author: adamgeva
 */

#ifndef B1RUN_HH_
#define B1RUN_HH_

#include "globals.hh"
#include "params.hh"
#include "G4Run.hh"

#include "G4THitsMap.hh"

class G4Event;
class B1Run : public G4Run
{
public:
	B1Run();
  virtual ~B1Run();

  virtual void RecordEvent(const G4Event*);
  virtual void Merge(const G4Run*);

  G4double GetTotalE(G4int scorerNum) const    { return GetTotal(fMapSum[scorerNum]); }

  G4double GetTotal(const G4THitsMap<G4double> &map) const;

  // Maps for accumulation
  G4THitsMap<G4double> fMapSum[NUM_OF_SCORERS];
  G4int fColIDSum[NUM_OF_SCORERS];


};


#endif /* B1RUN_HH_ */
