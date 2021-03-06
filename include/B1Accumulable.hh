/*
 * B1Accumulable.hh
 *
 *  Created on: Feb 20, 2018
 *      Author: adamgeva
 */

#ifndef B1ACCUMULABLE_HH_
#define B1ACCUMULABLE_HH_

#include "G4VAccumulable.hh"
#include "globals.hh"
#include "params.hh"
#include "globalFunctions.hh"
#include <map>

class B1Accumulable : public G4VAccumulable
{
 public:
	B1Accumulable(const G4String& name);

	virtual ~B1Accumulable();

	void updateP(G4int voxel_ind);
	void updateSm_hat(G4int voxel, G4int element, G4int detector, G4double value);
	void writeGradientAndP(G4int runNum);
	virtual void Merge(const G4VAccumulable& other);
	virtual void Reset();

	private:
    // gradient array - built per thread and holds the current gradient of all replicas (detector elements) w.r.t voxels and elements.
    G4double fSm_hat[NUM_OF_VOXELS][NUM_OF_ELEMENTS][NUM_OF_DETECTORS] = {};
    G4double fP[NUM_OF_VOXELS] = {};

};



#endif /* B1ACCUMULABLE_HH_ */
