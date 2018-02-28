/*
 * GradientKey.hh
 *
 *  Created on: Feb 28, 2018
 *      Author: adamgeva
 */

#ifndef GRADIENTKEY_HH_
#define GRADIENTKEY_HH_
class GradientKey {

public:
	GradientKey(G4int v,G4int el,G4int pix)
	:voxel(v), element(el), pixel(pix) {}
	G4int Getvoxel() const {return voxel;}
	G4int Getelement() const {return element;}
	G4int Getpixel() const {return pixel;}

	bool operator< (const GradientKey& GradientKeyObj) const
	{
		if(GradientKeyObj.pixel < this->pixel) return true;
		else if (GradientKeyObj.pixel > this->pixel) return false;
		else { //equal pixel
			if(GradientKeyObj.element < this->element) return true;
			else if (GradientKeyObj.element > this->element) return false;
			else { //equal element
				if(GradientKeyObj.voxel < this->voxel) return true;
				else if (GradientKeyObj.voxel > this->voxel) return false;
				else { //equal voxel
					return false;
				}
			}
		}
	}

private:
	G4int voxel;
	G4int element;
	G4int pixel;
};


#endif /* GRADIENTKEY_HH_ */
