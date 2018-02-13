/*
 * B1EnergyDeposit.cc
 *
 *  Created on: Apr 22, 2017
 *      Author: adamgeva
 */

#include "B1EnergyDeposit.hh"
#include "params.hh"
#include "globalFunctions.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4EmCalculator.hh"

extern B1EnergyDeposit* detectorsArray[NUM_OF_THREADS];


B1EnergyDeposit::B1EnergyDeposit(G4String name, G4int type)
//per thread
: G4PSEnergyDeposit(name)
{
	fscorerType =  type;
	if (type==0){ //randomly pick the 0 scorer
		G4int threadID = G4Threading::G4GetThreadId();
		if (threadID==-1) { //single threaded mode
			detectorsArray[0] = this;
		}
		else
		{
			detectorsArray[threadID+1] = this;
		}

	}
	//initialize gradient to zero
	//Sm_hat[NUM_OF_VOXELS][NUM_OF_ELEMENTS][NUM_OF_DETECTORS] = {};

}

B1EnergyDeposit::~B1EnergyDeposit()
{
// TODO: should we release the Sm_hat array?
}

G4int B1EnergyDeposit::GetIndex(G4Step* step){
	G4StepPoint* preStepPoint = step->GetPreStepPoint();
	//entering the detector
	G4TouchableHistory* touchable = (G4TouchableHistory*)(preStepPoint->GetTouchable());
	G4int ReplicaNum0 = touchable->GetReplicaNumber(0);
	G4int ReplicaNum1 = touchable->GetReplicaNumber(1);
	//G4int ReplicaNum2 = touchable->GetReplicaNumber(2);
	//10 is the number of bins
	return (ReplicaNum0 + ReplicaNum1*NUM_OF_ROWS);

}

G4bool B1EnergyDeposit::ProcessHits(G4Step* aStep,G4TouchableHistory* touchable){
	//test: getting information from tracks
	G4Track* track = aStep->GetTrack();
//	G4double trackWeight = track->GetWeight();


	//G4cout << "We have a hit, Weight: " << track->GetWeight() << G4endl;
//	G4String creatorProcessName = track->GetCreatorModelName();
//	G4int ParentID = track->GetParentID();
//	G4int ID = track->GetTrackID();
//	G4String type = track->GetDefinition()->GetParticleType();

	G4bool result = FALSE;
	G4VUserTrackInformation* info = track->GetUserInformation();
	B1TrackInformation* theInfo = (B1TrackInformation*)info;
	G4int totalNumOfInteractions = theInfo->GetNumberOfCompton() + theInfo->GetNumberOfRayl();

	//printing debug info:
//	G4cout << "track ID:" << track->GetTrackID() << G4endl;
//	G4cout << "track parent ID:" << track->GetParentID() << G4endl;
//	G4cout << "num of compt:" << theInfo->GetNumberOfCompton() << G4endl;
//	G4cout << "num of rayl:" << theInfo->GetNumberOfRayl() << G4endl;

	//very rare case in biasing - in case the photon underwent an interaction but wasnt split even tho it should have, example:
	//a photon hits the detector, potoelectric absorption and the an emittion of a new photon (ID 1 for example) then this photon undergoes Rayl in the phantom and arrives at the detector with weight 1.
	//TODO: check if correct!
	//not recording the main photons - only the virtuals
	if ((BIASING==1) && (track->GetParentID()==0)) {
		result = FALSE;
	}
	//G4cout << "TrackId = " << track->GetTrackID() << " number of total interactions = " << totalNumOfInteractions << G4endl;
	else if (fscorerType==1 || fscorerType==0){
		result = recordInteraction(aStep,touchable,totalNumOfInteractions,fscorerType);
	}
	else if(fscorerType==2){
		result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
	}
	else if(fscorerType==3){
		if ((theInfo->GetNumberOfCompton()<2) && (theInfo->GetNumberOfRayl()==0))
		{
			result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
		}
		else
		{
			result = FALSE;
		}
	}
	else if(fscorerType==4){
		if ((theInfo->GetNumberOfRayl()<2) && (theInfo->GetNumberOfCompton()==0)){
			result = G4PSEnergyDeposit::ProcessHits(aStep,touchable);
		}
		else
		{
			result = FALSE;
		}
	}
	// this scorer is in charge of writing the path file, could be any scorer.
	if(fscorerType==0){
		//energy deposited in the detector - equals to the final photon energy???
		G4double edep = aStep->GetTotalEnergyDeposit()/keV;
		//G4double final_energy = track->GetTotalEnergy();
		//G4double final_energy_kinetic = track->GetKineticEnergy();
		//holda the gradient of current photon segment, in the current voxel w.r.t all elements in that voxel
//	    G4StepPoint* preStepPoint = aStep->GetPreStepPoint();
//    	G4TouchableHistory* touchable2 = (G4TouchableHistory*)(preStepPoint->GetTouchable());
//		G4int ReplicaNum1 = touchable2->GetReplicaNumber(1);
		G4int detIndex = GetIndex(aStep);
	   	while (!theInfo->fpathLogList.empty()){
	   		segment seg = theInfo->fpathLogList.front();
	   		theInfo->fpathLogList.pop_front();
	   		if (CALC_GRADIENT == 1){
	   			updateGradTable(seg,edep,detIndex);
	   		}
	   		if(PRINT_PATHS == 1){
				//TODO: needs to be generalized - maybe there will be another process once!
				outputPathsFile << seg.voxel << "," << 0 << "," << seg.incidentEnergy/keV << "," << seg.pathLen << ",";
				//calc da_di_dvox - gradient of the attenuation w.r.t all the elements in the voxel
				//iterate over all participating elements and calc micXS
				//calc obj.ci = ((params.avogadro).*density)./(params.Ai);

				G4String enddd = seg.endingProcess;
				if (seg.endingProcess == "compt"){
					outputPathsFile << seg.voxel << "," << 1 << "," << seg.incidentEnergy/keV << "," << seg.scatteredEnergy/keV << ","; //<< seg.incidentEnergy/keV << "," << seg.scatteredEnergy/keV << ",";
				}
	   		}
	   	}
	   	if(PRINT_PATHS == 1){
	   		outputPathsFile << detIndex << "," << -100 << "\n";
	   	}
	}

	return result;
}

G4bool B1EnergyDeposit::recordInteraction (G4Step* aStep,G4TouchableHistory* touchable, G4int totalNumOfInteractions, G4int i) {
	if (totalNumOfInteractions>i){//recording all photons that did not interact in the phantom - transmission
		return FALSE;
	} else {
		//G4cout << "recording" << i << G4endl;
		return G4PSEnergyDeposit::ProcessHits(aStep,touchable);
	}
}

void B1EnergyDeposit::openFile(G4int threadNum,G4int runNum){
		std::string fileName =  std::string(OUTPUT_DIR) + "/" + IntToString(runNum) + "run" + IntToString(threadNum) + "paths.csv";
		outputPathsFile.open(fileName.c_str());
		//G4cout << "created file: " << fileName <<G4endl;
}

void B1EnergyDeposit::writeFile() {
	if (fscorerType==0){ // safty only
		//G4cout << "closing file: " <<G4endl;
		outputPathsFile.close();
	}
}

void B1EnergyDeposit::writeGradient(G4int threadNum,G4int runNum){
	//writing gradient table to file
	std::string fileName =  std::string(GRADIENT_DIR) + "/" + IntToString(runNum) + "run" + IntToString(threadNum) + "gradient.csv";
	std::ofstream outputPathsFile_grad;
	outputPathsFile_grad.open(fileName.c_str());
	for (G4int element = 0; element < NUM_OF_ELEMENTS; element ++){
		for (G4int voxel = 0; voxel < NUM_OF_VOXELS; voxel ++){
			for (G4int det = 0; det < NUM_OF_DETECTORS; det ++){
				outputPathsFile_grad << Sm_hat[voxel][element][det] << ',';
			}
			outputPathsFile_grad << '\n';
		}
	}
	outputPathsFile_grad.close();

	//write P array:
	fileName =  std::string(GRADIENT_DIR) + "/" + IntToString(runNum) + "run" + IntToString(threadNum) + "P.csv";
	std::ofstream outputPathsFile_P;
	outputPathsFile_P.open(fileName.c_str());
	for (G4int voxel = 0; voxel < NUM_OF_VOXELS; voxel ++){
		outputPathsFile_P <<  P[voxel] << ',';
	}
	outputPathsFile_P.close();
}

// methods for grad calculations
G4double B1EnergyDeposit::getTotalMicXS(G4Element* el, G4double Energy){
	// currently dealing with compton and photoelectric absorption only
	G4EmCalculator emCalculator;
	G4double comptXS;
	G4double photXS;
	photXS = emCalculator.ComputeCrossSectionPerAtom(Energy,"gamma","phot",el,0)/cm2;
	comptXS = emCalculator.ComputeCrossSectionPerAtom(Energy,"gamma","compt",el,0)/cm2;
	return photXS + comptXS;
}


G4double B1EnergyDeposit::getComptonMicDifferentialXS(G4Element* el, G4double E0 , G4double E1){
	G4double electron_mass_c2 = 510.99906 * keV;
	G4double classic_electr_radius = 2.818e-13 * cm;
	G4double classic_electr_radius2 = classic_electr_radius * classic_electr_radius;
	G4double eps = E1/E0;
	G4double eps_round = round( eps * 10000.0 ) / 10000.0; //rounds to 4 decimal points
	G4double deps = 1e-4; //resolution
	G4double Z = el->GetZ();
	G4double cost = 1 - electron_mass_c2/(eps_round*E0) + electron_mass_c2/E0;
	G4double cost2 = cost * cost;
	G4double sint2 = 1 - cost2;
	G4double f = 1/eps_round + eps_round;
	G4double q = 1 - (eps_round * sint2)/(1 + eps_round * eps_round);
	G4double dsigma_deps = M_PI * classic_electr_radius2 * (electron_mass_c2/E0) * f * q * Z / (2 * M_PI);
	return dsigma_deps * deps;
}

G4double B1EnergyDeposit::getComptonMacDifferentialXS(G4Material* mat, G4double E0 , G4double E1){
	const G4ElementVector* curr_element_vector = mat->GetElementVector();
	//vector of number of atoms per volume
	const G4double* curr_num_of_atoms =  mat->GetVecNbOfAtomsPerVolume();
	G4int nElements = mat->GetNumberOfElements();
	//calc dsigma macroscopic:
	G4double dsigmaMac = 0;
	for (G4int i=0 ; i<nElements ; i++) {
		dsigmaMac = dsigmaMac + curr_num_of_atoms[i]/(1/cm3) * getComptonMicDifferentialXS((*curr_element_vector)[i], E0 , E1);
	}
	return dsigmaMac;
}

void B1EnergyDeposit::updateGradTable(segment seg, G4double final_energy, G4int detIndex){
	//calc da_di_dvox - gradient of the attenuation w.r.t all the elements in the voxel
	// - 1 to have the absolute voxel number in the phantom grid
	G4int curr_voxel = seg.voxel - 1;
	//we dont calculate the gradient of the air voxel
	if (curr_voxel == -1){
		return;
	}
	//update P array:
	P[curr_voxel]++;
	G4double curr_energy = seg.incidentEnergy/keV;
	G4double curr_len = seg.pathLen;
	G4double curr_density = seg.Mat->GetDensity()/(g/cm3);
	//vector of elements in current material/voxel
	const G4ElementVector* curr_element_vector = seg.Mat->GetElementVector();
	const G4double* curr_frac_vector = seg.Mat->GetFractionVector();
	//vector of number of atoms per volume
	const G4double* curr_num_of_atoms =  seg.Mat->GetVecNbOfAtomsPerVolume();
	G4int nElements = seg.Mat->GetNumberOfElements();
	//incase there is compton scatter - calc dsigma:
	if (seg.endingProcess == "compt"){
		//calc dsigma macroscopic:
		G4double dsigmaMac = getComptonMacDifferentialXS(seg.Mat,seg.incidentEnergy/keV,seg.scatteredEnergy/keV);
		// iterate over elements and calc attenuation factor and scatter:
		for (G4int i=0 ; i<nElements ; i++) {
				G4double n_i = curr_num_of_atoms[i]/(1/cm3);
				G4double frac_i = curr_frac_vector[i];
				G4double N_by_A = n_i / (curr_density * frac_i);
				G4Element* el_i =  (*curr_element_vector)[i];
				G4double dsigma_di_dvox = N_by_A * getComptonMicDifferentialXS(el_i,seg.incidentEnergy/keV,seg.scatteredEnergy/keV) * (1/dsigmaMac);
				G4double da_di_dvox = -1 * curr_len * N_by_A * getTotalMicXS(el_i,curr_energy*keV);
				//update gradient:
				Sm_hat[curr_voxel][i][detIndex] = Sm_hat[curr_voxel][i][detIndex] + final_energy * (da_di_dvox + dsigma_di_dvox);
		}
	} else { //no compton at the end
		for (G4int i=0 ; i<nElements ; i++) {
				G4double n_i = curr_num_of_atoms[i]/(1/cm3);
				G4double frac_i = curr_frac_vector[i];
				G4double N_by_A = n_i / (curr_density * frac_i);
				G4Element* el_i =  (*curr_element_vector)[i];
				std::string el_name = el_i->GetName();
				//G4double micmic = getTotalMicXS(el_i,curr_energy*keV);
				G4double da_di_dvox = -1 * curr_len * N_by_A * getTotalMicXS(el_i,curr_energy*keV);
				//update gradient:
				Sm_hat[curr_voxel][i][detIndex] = Sm_hat[curr_voxel][i][detIndex] + final_energy * (da_di_dvox);
		}
	}
}
