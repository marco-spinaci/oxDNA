/*
 * PatchyInteraction.h
 *
 *  Created on: 14/mar/2013
 *      Author: lorenzo
 */

#ifndef PATCHYINTERACTION_H_
#define PATCHYINTERACTION_H_

#define PATCHY_POWER 200

#include "BaseInteraction.h"

/**
 * @brief Manages the interaction between simple patchy particles (as described in http://jcp.aip.org/resource/1/jcpsa6/v131/i1/p014504_s1)
 *
 * This interaction is selected with
 * interaction_type = patchy
 *
 * @verbatim
PATCHY_N = <int> (number of patches)
[PATCHY_N_B = <int> (number of patches on species B)]
[PATCHY_alpha = <float> (width of patches, defaults to 0.12)]
[PATCHY_epsilon_AA = <float> (depth of the well of the patch-patch interaction between particles of species A)]
[PATCHY_epsilon_BB = <float> (depth of the well of the patch-patch interaction between particles of species B)]
[PATCHY_epsilon_AB = <float> (depth of the well of the patch-patch interaction between particles of different species)]
[PATCHY_sigma_AA = <float> (diameter controlling the repulsive interaction between particles of species A)]
[PATCHY_sigma_BB = <float> (diameter controlling the repulsive interaction between particles of species B)]
[PATCHY_sigma_AB = <float> (diameter controlling the repulsive interaction between particles of different species)]
@endverbatim
 */

class PatchyInteraction: public BaseInteraction<PatchyInteraction > {
protected:
	/// Number of patches per particle
	int _N_patches;

	/// Number of patches per second-species particle
	int _N_patches_B;

	/// Number of particles per species
	int _N_A, _N_B;

	/// True if we are to simulate a patchy binary mixture, false otherwise
	bool _is_binary;

	/// Particles' diameters
	number _sigma[3];

	/// Squared diameters
	number _sqr_sigma[3];

	/// Repulsive interaction cut-off squared
	number _sqr_tot_rcut[3];

	/// Depth of the patch-patch well
	number _epsilon[3];

	/// Repulsive interaction energy at the cut-off
	number _E_cut[3];

	/// Patch-patch interaction energy at the cut-off
	number _patch_E_cut[3];

	/// Patch-patch interaction cut-off squared
	number _sqr_patch_rcut;

	/// Width of the patch, defaults to 0.12
	number _patch_alpha;

	/// _patch_alpha^10
	number _patch_pow_alpha;

	/**
	 * @brief Patchy interaction between two particles.
	 *
	 * @param p
	 * @param q
	 * @param r
	 * @param update_forces
	 * @return
	 */
	inline number _patchy_interaction(BaseParticle *p, BaseParticle *q, bool compute_r, bool update_forces);

public:
	enum {
		PATCHY = 4
	};

	PatchyInteraction();
	virtual ~PatchyInteraction();

	virtual void get_settings(input_file &inp);
	virtual void init();

	number get_alpha() { return _patch_alpha; }

	virtual void allocate_particles(std::vector<BaseParticle *> &particles);

	virtual number pair_interaction(BaseParticle *p, BaseParticle *q, bool compute_r = true, bool update_forces=false);
	virtual number pair_interaction_bonded(BaseParticle *p, BaseParticle *q, bool compute_r = true, bool update_forces=false);
	virtual number pair_interaction_nonbonded(BaseParticle *p, BaseParticle *q, bool compute_r = true, bool update_forces=false);
	virtual number pair_interaction_term(int name, BaseParticle *p, BaseParticle *q, bool compute_r = true, bool update_forces=false) {
		return this->_pair_interaction_term_wrapper(this, name, p, q, compute_r, update_forces);
	}

	virtual void read_topology(int *N_strands, std::vector<BaseParticle *> &particles);
	virtual void check_input_sanity(std::vector<BaseParticle *> &particles);
};


number PatchyInteraction::_patchy_interaction(BaseParticle *p, BaseParticle *q, bool compute_r, bool update_forces) {
	number sqr_r = _computed_r.norm();
	int type = p->type + q->type;
	if(sqr_r > _sqr_tot_rcut[type]) return (number) 0.f;

	number energy = (number) 0.f;

	number part = powf(_sqr_sigma[type]/sqr_r, PATCHY_POWER*0.5f);
	energy = part - _E_cut[type];

	if(update_forces) {
		LR_vector force = _computed_r * (PATCHY_POWER*part/sqr_r);
		p->force -= force;
		q->force += force;
	}

	int c = 0;
	LR_vector tmptorquep(0, 0, 0);
	LR_vector tmptorqueq(0, 0, 0);
	for(auto &ppatch: p->int_centers) {
		for(auto &qpatch: q->int_centers) {
			LR_vector patch_dist = _computed_r + qpatch - ppatch;
			number dist = patch_dist.norm();
			if(dist < _sqr_patch_rcut) {
				c++;
				number r8b10 = dist*dist*dist*dist / _patch_pow_alpha;
				number exp_part = -1.001f*_epsilon[type]*exp(-(number)0.5f*r8b10*dist);

				energy += exp_part - _patch_E_cut[type];

				if(update_forces) {
					LR_vector tmp_force = patch_dist * (5*exp_part*r8b10);

					p->torque -= p->orientationT*ppatch.cross(tmp_force);
					q->torque += q->orientationT*qpatch.cross(tmp_force);

					p->force -= tmp_force;
					q->force += tmp_force;
				}
			}
		}
	}

	return energy;
}

#endif /* PATCHYINTERACTION_H_ */
