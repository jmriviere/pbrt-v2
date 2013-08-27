/*
 * importance.h
 *
 *  Created on: 22 Jul 2013
 *      Author: jmr12
 */

#ifndef IMPORTANCE_H_
#define IMPORTANCE_H_

typedef struct s_d1d {
	uint offset;
	float integral;
	uint count;
} Distribution1D;

__constant float4 weight = (float4)(0.212671f, 0.715160f, 0.072169f, 0.f);

float2 sampleContinuous2D(float u1, float u2, __global const Distribution1D* pConditionalV,
		Distribution1D pMarginal, __global const float* cdfConditionalV,
		__global const float* cdfMarginal, __global const float* fun2D,
		__global const float* fun1D, float* pdf);

float sampleContinuous1D(float u, Distribution1D distribution, __global const float* cdf,
		__global const float* func, int* offset, float* pdf);

#endif /* IMPORTANCE_H_ */
