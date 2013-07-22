/*
 * importance.cl
 *
 *  Created on: 22 Jul 2013
 *      Author: jmr12
 */

#include "GPU.h"
#include "importance.h"

__kernel void init_luminance_pwc(__read_only image2d_t env, __global float* pw_lum,
		uint width, uint height) {

	uint x = get_global_id(0);
	uint y = get_global_id(1);

	Color lum = read_imagef(env, sampler, (float2)(x/(float)width,y/(float)height));
	pw_lum[y * width + x] = dot(lum, weight);
}

__kernel void init_Distribution1D(__global float* func, __global float* cdf,
		__global Distribution1D* pdf, uint n) {
	uint id =  get_global_id(0);
	uint offset = n * id;

	Distribution1D d1d;
    d1d.count = n;
    d1d.offset = offset;
    // Compute integral of step function at $x_i$
    cdf[offset] = 0.f;
    for (uint i = 1; i < n+1; ++i) {
        cdf[offset + i] = cdf[offset + i-1] + func[offset + i-1] / n;
    }

    // Transform step function integral into CDF
    d1d.integral = cdf[offset + n];
    if (d1d.integral == 0.f) {
        for (uint i = 1; i < n+1; ++i)
            cdf[offset + i] = (float)i / (float)n;
    }
    else {
        for (int i = 1; i < n+1; ++i)
            cdf[offset + i] /= d1d.integral;
    }
    pdf[id] = d1d;
}

// Look into atomic operations and synchronization
__kernel void init_Distribution1DCopy(__global Distribution1D* toCopy, __global float* cdf,
		__global Distribution1D* pdf, uint n) {
	uint id =  get_global_id(0); // Should always be 0
	uint offset = n * id; // Should always be 0

	Distribution1D d1d;
    d1d.count = n;
    d1d.offset = offset;
    // Compute integral of step function at $x_i$
    cdf[offset] = 0.f;
    for (uint i = 1; i < n+1; ++i) {
        cdf[offset + i] = cdf[offset + i-1] + toCopy[offset + i-1].integral / n;
        printf("%g ", cdf[offset + i-1]);
    }

    // Transform step function integral into CDF
    d1d.integral = cdf[offset + n];
    if (d1d.integral == 0.f) {
        for (uint i = 1; i < n+1; ++i)
            cdf[offset + i] = (float)i / (float)n;
    }
    else {
        for (int i = 1; i < n+1; ++i)
            cdf[offset + i] /= d1d.integral;
    }
    *pdf = d1d;
}

//__kernel void init_pdf(__global float* pw_lum, __global Distribution2D* d2d,
//		uint width, uint height) {
//	Distribution1D()
//}
