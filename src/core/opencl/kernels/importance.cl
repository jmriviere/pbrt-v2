/*
 * importance.cl
 *
 *  Created on: 22 Jul 2013
 *      Author: jmr12
 */

#include "GPU.h"
#include "importance.h"

__kernel void init_luminance_pwc(image2d_t env, __global float* pw_lum,
		uint width, uint height) {

	uint x = get_global_id(0);
	uint y = get_global_id(1);

	Color lum = read_imagef(env, sampler, (float2)(x/(float)width,y/(float)height));
	pw_lum[y * width + x] = dot(lum, weight);
}

__kernel void init_Distribution2D(__global const float* func,  uint n,
		__global float* cdfV, __global Distribution1D* d1dV, __global float* funcRed) {
	uint id =  get_global_id(0);
	uint offset = n * id;

	Distribution1D d1d_array_entry;
	d1d_array_entry.count = n;
	d1d_array_entry.offset = offset;
    // Compute integral of step function at $x_i$
	cdfV[offset] = 0.f;
    for (uint i = 1; i < n+1; ++i) {
        cdfV[offset + i] = cdfV[offset + i-1] + func[offset + i-1] / n;
    }

    // Transform step function integral into CDF
    d1d_array_entry.integral = cdfV[offset + n];
    if (d1d_array_entry.integral == 0.f) {
        for (uint i = 1; i < n+1; ++i)
            cdfV[offset + i] = (float)i / (float)n;
    }
    else {
        for (uint i = 1; i < n+1; ++i)
            cdfV[offset + i] /= d1d_array_entry.integral;
    }
    d1dV[id] = d1d_array_entry;
    if (funcRed) funcRed[id] = d1d_array_entry.integral;
}

// Binary search!!
static inline int upper_bound(float u, __global const float* cdf, int size) {
	int imin = 0;
	int imax = size - 1;

	// continue searching while [imin,imax] is not empty
	while (imax >= imin) {
		/* calculate the midpoint for roughly equal partition */
		int imid = imin + (imax-imin)/2;

		// determine which subarray to search
		if (cdf[imid] < u) {
			// change min index to search upper subarray
			imin = imid + 1;
		}
		else if (cdf[imid] > u) {
			// change max index to search lower subarray
			imax = imid - 1;
		}
		else
			// The value directly superior to u is at imid+1
			return imid;
	}
	return imax;
}

inline float2 sampleContinuous2D(float u1, float u2, __global const Distribution1D* pConditionalV,
		Distribution1D pMarginal, __global const float* cdfConditionalV,
		__global const float* cdfMarginal, __global const float* fun2D,
		__global const float* fun1D, float* pdf) {
	int v,vv;
	float pdfs[2];
	float s1 = max(sampleContinuous1D(u1, pMarginal, cdfMarginal, fun1D, &v, &pdfs[0]), 0.f);
	float s2 = max(sampleContinuous1D(u2, pConditionalV[v], cdfConditionalV, fun2D, &vv, &pdfs[1]),
			0.f);
	float2 sample = (float2)(s1, s2);
	*pdf = pdfs[0] * pdfs[1];
	return sample;
}

float sampleContinuous1D(float u, Distribution1D distribution, __global const float* cdf,
		__global const float* func, int* offset, float* pdf) {
	int off = upper_bound(u, &cdf[distribution.offset], distribution.count);
	*offset = off;
	// Compute offset along CDF segment
	float du = (u - cdf[distribution.offset + off]) /
			(cdf[distribution.offset + off+1] - cdf[distribution.offset + off]);
	*pdf = func[distribution.offset + off] / distribution.integral;
	return (off + du) / distribution.count;
}
