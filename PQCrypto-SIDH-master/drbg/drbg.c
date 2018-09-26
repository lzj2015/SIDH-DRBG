#include <stdio.h>

#include "drbg.h"
#include "sha256.h"
#include <string.h>
#include <stdlib.h>

extern const uint64_t A_gen[5 * NWORDS64_FIELD];
extern const uint64_t B_gen[5 * NWORDS64_FIELD];
extern const uint64_t Montgomery_one[NWORDS64_FIELD];
extern const unsigned int strat_Alice[MAX_Alice - 1];
extern const unsigned int strat_Bob[MAX_Bob - 1];


#define fpcopy                        fpcopy503
#define fpzero                        fpzero503
#define fpadd                         fpadd503
#define fpsub                         fpsub503
#define fpneg                         fpneg503
#define fpdiv2                        fpdiv2_503
#define fpcorrection                  fpcorrection503
#define fpmul_mont                    fpmul503_mont
#define fpsqr_mont                    fpsqr503_mont
#define fpinv_mont                    fpinv503_mont
#define fpinv_chain_mont              fpinv503_chain_mont
#define fpinv_mont_bingcd             fpinv503_mont_bingcd
#define fp2copy                       fp2copy503
#define fp2zero                       fp2zero503
#define fp2add                        fp2add503
#define fp2sub                        fp2sub503
#define fp2neg                        fp2neg503
#define fp2div2                       fp2div2_503
#define fp2mul_mont                   fp2mul503_mont
#define fp2sqr_mont                   fp2sqr503_mont
#define fp2inv_mont                   fp2inv503_mont
#define fp2correction                 fp2correction503


inline static void init_gens(digit_t *gen, f2elm_t XP, f2elm_t XQ, f2elm_t XR)
{ // Initialization of basis points

	fpcopy503(gen, XP[0]);
	fpcopy503(gen + NWORDS_FIELD, XP[1]);
	fpcopy503(gen + 2 * NWORDS_FIELD, XQ[0]);
	fpzero503(XQ[1]);
	fpcopy503(gen + 3 * NWORDS_FIELD, XR[0]);
	fpcopy503(gen + 4 * NWORDS_FIELD, XR[1]);
}

inline static void j_inv24(const f2elm_t A24, const f2elm_t C24, f2elm_t jinv)
{
	f2elm_t A, C;
	fp2div2_503(C24, C);
	fp2sub503(A24, C, A);
	fp2div2_503(C, C);
	j_inv(A, C, jinv);

}

inline static void modLAEA(const f2elm_t a, felm_t r)
{
	digit_t x;
	digit_t t[2 * NWORDS_FIELD] = {0};

	fpcopy503(a[0], t);
	fpcopy503(a[1], t+ NWORDS_FIELD);

	unsigned int limb_cnt = OALICE_BITS / sizeof(digit_t);

	x = t[limb_cnt] & ((1 << OALICE_BITS % sizeof(digit_t)) - 1);

	fpzero503(r);
	copy_words(t, r, limb_cnt);
	r[limb_cnt] = x;


}

static void DRBGp503_alice_gen(const DRBGp503_context *ctx)
{
	point_proj_t R, phiP = { 0 }, phiQ = { 0 }, phiR = { 0 }, pts[MAX_INT_POINTS_ALICE];
	f2elm_t coeff[3], A24plus = { 0 }, C24 = { 0 }, A = { 0 }, S = { 0 };
	felm_t Smod;
	unsigned int i, row, m, index = 0, pts_index[MAX_INT_POINTS_ALICE], npts = 0, ii = 0;


	fp2copy503(ctx->XP_B, phiP->X);
	fp2copy503(ctx->XQ_B, phiQ->X);
	fp2copy503(ctx->XR_B, phiR->X);
	fp2copy503(ctx->one, phiP->Z);
	fp2copy503(ctx->one, phiQ->Z);
	fp2copy503(ctx->one, phiR->Z);


	// Initialize constants
	fp2copy503(ctx->one, A24plus);
	fp2add(A24plus, A24plus, C24);


	LADDER3PT(ctx->XP_A, ctx->XQ_A, ctx->XR_A, (digit_t*)ctx->s, ALICE, R, A);


	// Traverse tree
	index = 0;
	for (row = 1; row < MAX_Alice; row++) {
		while (index < MAX_Alice - row) {
			fp2copy(R->X, pts[npts]->X);
			fp2copy(R->Z, pts[npts]->Z);
			pts_index[npts++] = index;
			m = strat_Alice[ii++];
			xDBLe(R, R, A24plus, C24, (int)(2 * m));
			index += m;
		}
		get_4_isog(R, A24plus, C24, coeff);

		for (i = 0; i < npts; i++) {
			eval_4_isog(pts[i], coeff);
		}
		eval_4_isog(phiP, coeff);
		eval_4_isog(phiQ, coeff);
		eval_4_isog(phiR, coeff);

		fp2copy(pts[npts - 1]->X, R->X);
		fp2copy(pts[npts - 1]->Z, R->Z);
		index = pts_index[npts - 1];
		npts -= 1;
	}

	get_4_isog(R, A24plus, C24, coeff);
	j_inv24(A24plus, C24, S);

	//S mod l_A^E_A
	modLAEA(S, Smod);
	memcpy(ctx->s, Smod,NBITS_TO_NBYTES(OALICE_BITS));

	
}

static void DRBGp503_bob_gen(const DRBGp503_context *ctx,unsigned int N,unsigned char *r,unsigned clen)
{
	point_proj_t R, phiP = { 0 }, phiQ = { 0 }, phiR = { 0 }, pts[MAX_INT_POINTS_BOB];
	f2elm_t coeff[3], A24plus = { 0 }, A24minus = { 0 }, A = { 0 }, C = { 0 }, jinv = {0};
	unsigned int i, row, m, index = 0, pts_index[MAX_INT_POINTS_BOB], npts = 0, ii = 0;

	fp2copy503(ctx->phBP_A, phiP->X);
	fp2copy503(ctx->phBQ_A, phiQ->X);
	fp2copy503(ctx->phBPQ_R, phiR->X);
	fp2copy503(ctx->one, phiP->Z);
	fp2copy503(ctx->one, phiQ->Z);
	fp2copy503(ctx->one, phiR->Z);

	// Initialize constants
	fp2copy503(ctx->EBC24, A24plus);
	fp2copy503(ctx->EBA24, A24minus);

	LADDER3PT(ctx->XP_B, ctx->XQ_B, ctx->XR_B, (digit_t*)ctx->s, BOB, R, ctx->EBAC);

	// Traverse tree
	index = 0;
	for (row = 1; row < MAX_Bob; row++) {
		while (index < MAX_Bob - row) {
			fp2copy(R->X, pts[npts]->X);
			fp2copy(R->Z, pts[npts]->Z);
			pts_index[npts++] = index;
			m = strat_Bob[ii++];
			xTPLe(R, R, A24minus, A24plus, (int)m);
			index += m;
		}
		get_3_isog(R, A24minus, A24plus, coeff);

		for (i = 0; i < npts; i++) {
			eval_3_isog(pts[i], coeff);
		}
		eval_3_isog(phiP, coeff);
		eval_3_isog(phiQ, coeff);
		eval_3_isog(phiR, coeff);

		fp2copy(pts[npts - 1]->X, R->X);
		fp2copy(pts[npts - 1]->Z, R->Z);
		index = pts_index[npts - 1];
		npts -= 1;
	}

	get_3_isog(R, A24minus, A24plus, coeff);



	fp2add503(A24plus, A24minus, A);
	fp2div2_503(A, A);
	fp2div2_503(A, A);
	fp2sub(A24plus, A, C);
	fp2div2_503(C,C);
	j_inv(A, C, jinv);

	
	memset(r, 0, clen);
	unsigned char t[2 * NWORDS_FIELD * sizeof(digit_t)] = { 0 };

	fpcopy503(jinv[0], (digit_t *)t);
	fpcopy503(jinv[1], (digit_t *)(t + NWORDS_FIELD * sizeof(digit_t)));


	unsigned int limb_cnt =  NBITS_TO_NBYTES(N);

	if (clen > 128)
	{
		clen = 128;
	}

	if (limb_cnt > 128) 
	{
		limb_cnt = 128;
	}

	
	if (clen>limb_cnt)
	{
		unsigned char x = t[limb_cnt] & ((1 << N % sizeof(unsigned char)) - 1);
		memcpy(r, t, limb_cnt);
		r[limb_cnt] = x;
	}
	else
	{
		memcpy(r, t, clen);
	}
}


void DRBGp503_context_init(DRBGp503_context *ctx)
{
	init_gens((digit_t*)A_gen, ctx->XP_A, ctx->XQ_A, ctx->XR_A);
	init_gens((digit_t*)B_gen, ctx->XP_B, ctx->XQ_B, ctx->XR_B);
	fpcopy503((digit_t*)&Montgomery_one,ctx->one[0]);
	fpzero503(ctx->one[1]);
	memset(ctx->s, 0, NBITS_TO_NBYTES(OALICE_BITS));
}

void DRBGp503_context_free(DRBGp503_context *ctx)
{
	fp2zero(ctx->phBP_A);
	fp2zero (ctx->phBQ_A);
	fp2zero (ctx->phBPQ_R);
	fp2zero(ctx->EBA24);
	fp2zero(ctx->EBC24);
	fp2zero(ctx->XP_A);
	fp2zero(ctx->XP_B);
	fp2zero(ctx->XQ_A);
	fp2zero(ctx->XQ_B);
	fp2zero(ctx->XR_A);
	fp2zero(ctx->XR_B);
	fp2zero(ctx->one);
	memset(ctx->s, 0, NBITS_TO_NBYTES(OALICE_BITS));
}


int DRBGp503_setup(DRBGp503_context *ctx, const unsigned char *s, unsigned int slen, unsigned int N)
{
	if (slen * 8 < OALICE_BITS)
	{
		return -1;
	}
	memcpy(ctx->s, s, NBITS_TO_NBYTES(OALICE_BITS));

	unsigned char out[32];
	sha256_context hash;
	point_proj_t R, phiP = { 0 }, phiQ = { 0 }, phiR = { 0 }, pts[MAX_INT_POINTS_BOB];
	f2elm_t coeff[3], A24plus = { 0 }, A24minus = { 0 }, A = { 0 }, C = { 0 };
	unsigned int i, row, m, index = 0, pts_index[MAX_INT_POINTS_BOB], npts = 0, ii = 0;


	sha256_init(&hash);
	sha256_starts(&hash, 0);
	sha256_update(&hash, s, slen);
	sha256_update(&hash, (unsigned char *)&N, sizeof(unsigned int));
	sha256_finish(&hash, out);
	sha256_free(&hash);

	out[32 - 1] &= MASK_BOB;


	fp2copy503(ctx->XP_A, phiP->X);
	fp2copy503(ctx->XQ_A, phiQ->X);
	fp2copy503(ctx->XR_A, phiR->X);
	fp2copy503(ctx->one, phiP->Z);
	fp2copy503(ctx->one, phiQ->Z);
	fp2copy503(ctx->one, phiR->Z);

	// Initialize constants
	fp2copy503(ctx->one, A24plus);
	fp2add(A24plus, A24plus, A24plus);
	fp2copy(A24plus, A24minus);
	fp2neg(A24minus);

	LADDER3PT(ctx->XP_B, ctx->XQ_B, ctx->XR_B, (digit_t*)out, BOB, R, A);


	// Traverse tree
	index = 0;
	for (row = 1; row < MAX_Bob; row++) {
		while (index < MAX_Bob - row) {
			fp2copy(R->X, pts[npts]->X);
			fp2copy(R->Z, pts[npts]->Z);
			pts_index[npts++] = index;
			m = strat_Bob[ii++];
			xTPLe(R, R, A24minus, A24plus, (int)m);
			index += m;
		}
		get_3_isog(R, A24minus, A24plus, coeff);

		for (i = 0; i < npts; i++) {
			eval_3_isog(pts[i], coeff);
		}
		eval_3_isog(phiP, coeff);
		eval_3_isog(phiQ, coeff);
		eval_3_isog(phiR, coeff);

		fp2copy(pts[npts - 1]->X, R->X);
		fp2copy(pts[npts - 1]->Z, R->Z);
		index = pts_index[npts - 1];
		npts -= 1;
	}


	get_3_isog(R, ctx->EBA24, ctx->EBC24, coeff);
	eval_3_isog(phiP, coeff);
	eval_3_isog(phiQ, coeff);
	eval_3_isog(phiR, coeff);

	inv_3_way(phiP->Z, phiQ->Z, phiR->Z);
	fp2mul_mont(phiP->X, phiP->Z, ctx->phBP_A);
	fp2mul_mont(phiQ->X, phiQ->Z, ctx->phBQ_A);
	fp2mul_mont(phiR->X, phiR->Z, ctx->phBPQ_R);

	fp2add503(A24plus, A24minus, A);
	fp2div2_503(A, A);
	fp2div2_503(A, A);

	fp2sub(A24plus, A, C);
	fp2div2_503(C, C);
	fp2inv_mont(C);
	fp2mul_mont(A, C, ctx->EBAC);

	return 0;
}



void DRBGp503_random(DRBGp503_context *ctx, unsigned int N, unsigned char *r, unsigned int rlen) 
{
	DRBGp503_alice_gen(ctx);
	DRBGp503_bob_gen(ctx, N, r, rlen);
}

