/*
 * PatchyParticle.h
 *
 *  Created on: 15/mar/2013
 *      Author: lorenzo
 */

#ifndef PATCHYPARTICLE_H_
#define PATCHYPARTICLE_H_

#include "BaseParticle.h"

/**
 * @brief Incapsulates a patchy particle with 2, 3, or 4 spherical patches. Used by PatchyInteraction.
 */

class PatchyParticle : public BaseParticle {
protected:
	number _sigma;
	std::vector<LR_vector> _base_patches;

public:
	PatchyParticle(std::vector<LR_vector> base_patches, int nt, number sigma);
	PatchyParticle(int N_patches, int nt, number sigma);
	virtual ~PatchyParticle();

	void set_positions();

	virtual bool is_rigid_body() {
		return true;
	}
};

#endif /* PATCHYPARTICLE_H_ */
