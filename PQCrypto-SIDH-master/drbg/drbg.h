
// create by lzj
// 18.9.4


#ifndef __TEST_EXTRAS_H__
#define __TEST_EXTRAS_H__



#include "../src/config.h"
#include "../src/P503/P503_internal.h"
#include "../src/P503/P503_api.h"



#define SCHEME_NAME    "DRBGp503"


typedef struct 
{
	f2elm_t phBP_A;
	f2elm_t phBQ_A;
	f2elm_t phBPQ_R;
	f2elm_t EBA24,EBC24;
	f2elm_t XP_A, XP_B, XQ_A, XQ_B,XR_A,XR_B;
	f2elm_t one;
	f2elm_t EBAC;
	unsigned char * s[NBITS_TO_NBYTES(OALICE_BITS)];

}DRBGp503_context;


void DRBGp503_context_init(DRBGp503_context *ctx);
void DRBGp503_context_free(DRBGp503_context *ctx);

int DRBGp503_setup(DRBGp503_context *ctx, const unsigned char *s, unsigned int slen, unsigned int N);

void DRBGp503_random(DRBGp503_context *ctx, unsigned int N, unsigned char *r, unsigned int rlen);


#endif