#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include "drbg.h"

int64_t cpucycles(void)
{ // Access system counter for benchmarking
#if (OS_TARGET == OS_WIN) && (TARGET == TARGET_AMD64 || TARGET == TARGET_x86)
	return __rdtsc();
#elif (OS_TARGET == OS_WIN) && (TARGET == TARGET_ARM)
	return __rdpmccntr64();
#elif (OS_TARGET == OS_LINUX) && (TARGET == TARGET_AMD64 || TARGET == TARGET_x86)
	unsigned int hi, lo;

	asm volatile ("rdtsc\n\t" : "=a" (lo), "=d"(hi));
	return ((int64_t)lo) | (((int64_t)hi) << 32);
#elif (OS_TARGET == OS_LINUX) && (TARGET == TARGET_ARM || TARGET == TARGET_ARM64)
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);
	return (int64_t)(time.tv_sec*1e9 + time.tv_nsec);
#else
	return 0;
#endif
}

void write()
{
	FILE *fp;
	unsigned int N = 256;
	unsigned char s[32];
	unsigned char r[32] = { 0 };
	unsigned int len = 32;

	if ((fp = fopen("tdrbg.dat", "wb")) == NULL) {
		printf("Cannot open file, press any key to exit!");
		return;
	}


	random_mod_order_A_SIDHp503(s);
	DRBGp503_context ctx;

	DRBGp503_context_init(&ctx);

	DRBGp503_setup(&ctx, s, len, N);
	unsigned int i;
	for ( i = 0; i < 10000000; i++)
	{
		DRBGp503_random(&ctx, N, r, len);
		fwrite(r, sizeof(char), len, fp);
		fflush(fp);
	}

	DRBGp503_context_free(&ctx);

	fclose(fp);
}

void testcycles() {
	int BENCH_LOOPS = 10000;
	unsigned int N = 256;
	unsigned char s[32];
	unsigned char r[64] = { 0 };
	unsigned int len = 32;
	int n;
	unsigned long long cycles, cycles1, cycles2;


	random_mod_order_A_SIDHp503(s);
	DRBGp503_context ctx;

	DRBGp503_context_init(&ctx);

	DRBGp503_setup(&ctx, s, len, N);


	printf("\n--------------------------------------------------------------------------------------------------------\n\n");

	cycles = 0;
	for (n = 0; n<BENCH_LOOPS; n++)
	{
		cycles1 = cpucycles();
		DRBGp503_random(&ctx, N, r, len);
		cycles2 = cpucycles();
		cycles = cycles + (cycles2 - cycles1);
	}
	printf("  256 runs in .......................................... %7lld cycles", cycles / BENCH_LOOPS);
	printf("\n");


	cycles = 0;
	for (n = 0; n<BENCH_LOOPS; n++)
	{
		cycles1 = cpucycles();
		DRBGp503_random(&ctx, N, r, len);
		cycles2 = cpucycles();
		cycles = cycles + (cycles2 - cycles1);
	}
	printf("  256*10000 runs in .......................................... %7lld cycles", cycles);
	printf("\n");


	len = 64;
	cycles = 0;
	for (n = 0; n<BENCH_LOOPS; n++)
	{
		cycles1 = cpucycles();
		DRBGp503_random(&ctx, N, r, len);
		cycles2 = cpucycles();
		cycles = cycles + (cycles2 - cycles1);
	}
	printf("  512 runs in .......................................... %7lld cycles", cycles / BENCH_LOOPS);
	printf("\n");

	DRBGp503_context_free(&ctx);
}



int main()
{
	
	FILE *fp;
	unsigned int N = 256;
	unsigned char s[32];
	unsigned char r[32] = { 0 };
	unsigned int len = 32;


	if ((fp = fopen("tdrbg.txt", "wb")) == NULL) {
		printf("Cannot open file, press any key to exit!");
		return;
	}

	random_mod_order_A_SIDHp503(s);
	DRBGp503_context ctx;

	DRBGp503_context_init(&ctx);

	DRBGp503_setup(&ctx, s, len, N);
	unsigned int i;
	for (i = 0; i < 10000; i++)
	{
		DRBGp503_random(&ctx, N, r, len);
		for (size_t j = 0; j < len; j++)
		{
			fprintf(fp, "%02x", r[j]);
		}
		fprintf(fp, "\n");
		fflush(fp);
	}

	DRBGp503_context_free(&ctx);
	

	fclose(fp);
	return 0;
}