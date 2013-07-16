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


#define OFFSETOF(type, field)    ((unsigned long) &(((type *) 0)->field))

//#pragma OPENCL EXTENSION cl_amd_printf : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
							   CLK_ADDRESS_CLAMP_TO_EDGE |
							   CLK_FILTER_LINEAR;

typedef enum GPUType {
	sphere = 0,
	light = 1
} GPUType;

typedef enum GPUMaterial {
	DIFF = 0,
	SPEC = 1,
	REFR = 2
} GPUMaterial;

typedef struct {
	float4 m[4];
} Transformation;


typedef struct __attribute__ ((packed)) s_metadata {
	Transformation toWorld;
	Transformation fromWorld;
	GPUType type;
	uint offset;
	union {
		uint2 dim;
		GPUMaterial mat;
	};
} Metadata;

typedef struct __attribute__ ((packed)) s_ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct s_sphere {
	float radius;
} Sphere;

typedef struct Hit {
	int id;
	float t;
} Hit;

typedef float4 Color;

inline float3 transform_vect(float3 r, Transformation t) {
	float3 ret;

	ret.s0 = dot(t.m[0], (float4)(r, 0));
	ret.s1 = dot(t.m[1], (float4)(r, 0));
	ret.s2 = dot(t.m[2], (float4)(r, 0));

	return ret;
}

inline float3 transform_point(float3 r, Transformation t) {
	float3 ret;

	ret.s0 = dot(t.m[0], (float4)(r, 1));
	ret.s1 = dot(t.m[1], (float4)(r, 1));
	ret.s2 = dot(t.m[2], (float4)(r, 1));
	float w = dot(t.m[3], (float4)(r, 1));

	if (w != 1) {
		ret /= w;
	}

	return ret;
}

#endif /* GPU_H_ */
