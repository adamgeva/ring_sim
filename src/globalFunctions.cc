/*
 * globalFunctions.cc

 *
 *  Created on: Apr 17, 2017
 *      Author: adamgeva
 */

#include "globalFunctions.hh"
#include <stdlib.h>
#include "params.hh"
#include <math.h>

#define PI 3.14159265


//todo: remove from here to a macro!!!
std::string IntToString (int a)
{
    std::ostringstream temp;
    temp<<a;
    return temp.str();
}
//russian roulette
//p is probability to survive. return true if particle survives and false otherwise
G4bool RR (G4double p)
{
	G4double sample = rand() / (RAND_MAX + 1.);
	if (sample < p) return true;
	return false;
}

//Check in or out of ring - true if out
G4bool outOfRing (G4ThreeVector position, G4ThreeVector momentumDirection, G4double Zup , G4double Zdown, G4double ringRadius) {
	G4double dx = momentumDirection[0];
	G4double dy = momentumDirection[1];
	G4double dz = momentumDirection[2];
	G4double x = position[0];
	G4double y = position[1];
	G4double z = position[2];
	G4double alpha = 1/(sqrt(pow(dx,2)+pow(dy,2)));

	//normalize to unit size vector in XY plane by multiplying by alpha
	G4double dxNorm = dx*alpha;
	G4double dyNorm = dy*alpha;
	//calc angles
	G4double theta1 = atan (abs(y)/abs(x)) * 180 / PI;
	//G4double theta2 = 90 - theta1;
	G4double theta3 = atan (abs(dyNorm)/abs(dxNorm)) * 180 / PI;
	G4double gamma = theta1 + theta3;
	//calc ll
	G4double ll = (ringRadius/sin(gamma)) * sin(90-theta1);
	//find intersection on the ring
	G4double xRing = x + ll*dxNorm;
	G4double yRing = y + ll*dyNorm;
	//up ring vector - from current photon position to up ring
	G4double zRing = Zup;
	G4double vecx = xRing - x;
	G4double vecy = yRing - y;
	G4double vecz = zRing - z;
	G4double upAngle = atan(vecz/(sqrt(pow(vecx,2)+pow(vecy,2)))) * 180 / PI;
	//down ring vector
	zRing = Zdown;
	vecx = xRing - x;
	vecy = yRing - y;
	vecz = zRing - z;
	G4double downAngle = atan(abs(vecz)/(sqrt(pow(vecx,2)+pow(vecy,2)))) * 180 / PI;
	//calc current angle
	G4double currentAngle = atan(abs(dz)/(sqrt(pow(dx,2)+pow(dy,2)))) * 180 / PI;
	//check if out bound or not
	if (currentAngle>upAngle || currentAngle<downAngle) {
		return true; //out
	}
	return false;
}
//G4ThreeVector Mom = actualParticleChange->GetProposedMomentumDirection();
//G4cout << "Mom = " << Mom << G4endl;
//G4cout << "MomX = " << Mom[0] << " MomY = " << Mom[1] << " MomZ= " << Mom[2] << G4endl;
//G4cout << "Mom RMS = " << sqrt(pow(Mom[0],2) + pow(Mom[1],2) + pow(Mom[2],2)) << G4endl;
