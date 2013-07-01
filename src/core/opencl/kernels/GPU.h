/*
 * GPU.h
 *
 *  Created on: 27 Jun 2013
 *      Author: jmr12
 */

#ifndef GPU_H_
#define GPU_H_

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#pragma OPENCL EXTENSION cl_amd_printf : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
							   CLK_ADDRESS_CLAMP_TO_EDGE |
							   CLK_FILTER_LINEAR;

typedef enum GPUType {
	sphere = 0,
	light = 1
} GPUType;

typedef struct {
	float4 m[4];
} Transformation;


typedef struct __attribute__ ((packed)) s_metadata {
	GPUType type;
	uint offset;
	Transformation toWorld;
	Transformation fromWorld;
	float2 dim;
} Metadata;

typedef struct __attribute__ ((packed)) s_ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct s_hit {
	Ray ray;
	float t;
} Hit;

typedef struct s_sphere {
	float radius;
} Sphere;

typedef float4 Color;

#endif /* GPU_H_ */
