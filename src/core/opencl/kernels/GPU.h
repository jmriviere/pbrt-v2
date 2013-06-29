/*
 * GPUshapes.h
 *
 *  Created on: 27 Jun 2013
 *      Author: jmr12
 */

#ifndef GPU_H_
#define GPU_H_

#define M_PI           3.14159265358979323846

#pragma OPENCL EXTENSION cl_amd_printf : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
							   CLK_ADDRESS_CLAMP_TO_EDGE |
							   CLK_FILTER_LINEAR;

typedef struct {
	float4 m[4];
} Transformation;

__constant Transformation w2l = { {(float4)(1,0,0,0), (float4)(0,-4.37114e-08,-1,0),(float4)(0,1,-4.37114e-08,0),(float4)(0,0,0,1)} };

typedef struct __attribute__ ((packed)) s_ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct s_hit {
	Ray ray;
	float t;
} Hit;

typedef struct __attribute__ ((packed)) s_sphere {
	Transformation o2w;
	Transformation w2o;
	float radius;
} Sphere;

typedef float4 Color;

#endif /* GPU_H_ */
