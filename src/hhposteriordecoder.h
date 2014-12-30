/*
 * hhposteriordecoder.h
 *
 *  Created on: 19/02/2014
 *      Author: Stefan Haunsberger
 *
 *  Copyright (c) 2014 -. All rights reserved.
 *
 *  Info:
 *		This class contains the needed algorithms to perform the
 *		MAC algorithm:
 *			+ Forward (SIMD),
 *			+ Backward (SIMD),
 *			+ MAC (SIMD) and
 *			+ MAC backtrace (scalar).
 *
 *		The realign method is called by the posterior consumer thread.
 *		It prepares all needed matrices and parameters. This includes the
 *			- exclusion of previously found alignments
 *				(found by Viterbi or MAC) and the current Viterbi alignment
 *			- memorize selected hit values (Eval, Pval,...) that are restored
 *				after the posterior decoding, MAC and backtrace.
 *
 *		After the preparation step the algorithms in the above represented
 *		order are computed.
 *
 */

#ifndef HHPOSTERIORDECODER_H_
#define HHPOSTERIORDECODER_H_
#include <map>
#include <vector>
#include <stddef.h>

#include "hhhmmsimd.h"
#include "hhviterbimatrix.h"
#include "hhposteriormatrix.h"
#include "hhviterbi.h"
using std::map;

class PosteriorDecoder {
public:

	static const int VEC_SIZE = HMMSimd::VEC_SIZE;

	struct MACBacktraceResult {
		std::vector<int> * alt_i;
		std::vector<int> * alt_j;
		MACBacktraceResult(std::vector<int> * alt_i, std::vector<int> * alt_j):
				alt_i(alt_i), alt_j(alt_j) {};
	};

	PosteriorDecoder(int maxres, bool local, int q_length);

	virtual ~PosteriorDecoder();

	/////////////////////////////////////////////////////////////////////////////////////
	// Realign hits: compute F/B/MAC and MAC-backtrace algorithms SIMD
	/////////////////////////////////////////////////////////////////////////////////////
	void realign(HMM &q, HMM &t, Hit &hit, PosteriorMatrix &p_mm, ViterbiMatrix &viterbi_matrix, int par_min_overlap, float shift, float mact, float corr);
	void excludeMACAlignment(const int q_length, const int t_length, ViterbiMatrix &celloff_matrix, const int elem,
			MACBacktraceResult & alignment);

private:

	float * m_mm_prev;
	float * m_gd_prev;
	float * m_dg_prev;
	float * m_im_prev;
	float * m_mi_prev;

	float * m_mm_curr;
	float * m_gd_curr;
	float * m_dg_curr;
	float * m_im_curr;
	float * m_mi_curr;

	float * m_s_curr;		// MAC scores - current
	float * m_s_prev;		// MAC scores - previous
	float * p_last_col;

	float * m_backward_profile;
	float * m_forward_profile;

	double * scale;

//	PosteriorSharedVariables m_column_vars;

//	simd_float m_p_min;    // used to distinguish between SW and NW algorithms in maximization
	float m_p_min_scalar;    // used to distinguish between SW and NW algorithms in maximization

//	std::vector<Hit *> m_temp_hit;	// temporary used hit objects for computation

	const int m_max_res;
	const bool m_local;				// local alignment
	const int m_q_length;			// query length

	int m_jmin;

	simd_float * m_p_forward;

	Hit * m_temp_hit;

	void forwardAlgorithm(HMM & q_hmm, HMM & t_hmm, Hit & hit_vec, PosteriorMatrix & p_mm,
			ViterbiMatrix & viterbi_matrix, float shift, const int elem);
	void backwardAlgorithm(HMM & q_hmm,HMM & t_hmm, Hit & hit_vec, PosteriorMatrix & p_mm,
			ViterbiMatrix & viterbi_matrix, float shift, const int elem);
	void macAlgorithm(HMM & q_hmm, HMM & t_hmm, Hit & hit_vec, PosteriorMatrix & p_mm,
			ViterbiMatrix & viterbi_matrix, float par_mact, const int elem);
	void backtraceMAC(HMM & q, HMM & t, PosteriorMatrix & p_mm, ViterbiMatrix & backtrace_matrix, const int elem, Hit & hit, float corr);
	void writeProfilesToHits(HMM &q, HMM &t, PosteriorMatrix &p_mm, Hit &hit);
	void initializeBacktrace(HMM & t, Hit & hit);

	void initializeForAlignment(HMM &q, HMM &t, Hit &hit, ViterbiMatrix &viterbi_matrix, const int elem, const int t_max_L, int par_min_overlap);
	void maskViterbiAlignment(const int q_length, const int t_length, ViterbiMatrix &celloff_matrix,
			const int elem, Hit const &hit) const;
	void memorizeHitValues(Hit & curr_hit);
	void restoreHitValues(Hit &curr_hit);

	void setGlobalColumnPForward(simd_float * column, const simd_int & j_vec, const int i_count, const simd_float & values);

	void printVector(simd_float * vec);
	void printVector(simd_int * vec);
	void printVector(float * vec);

};

#endif /* HHPOSTERIORDECODER_H_ */
