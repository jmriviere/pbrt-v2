/*
 * ray_caster.h
 *
 *  Created on: 5 Jul 2013
 *      Author: jmr12
 */

#ifndef RAY_CASTER_H_
#define RAY_CASTER_H_

#include "importance.h"
#include "OCL.h"
#include "fresnel.h"
#include "camera.h"

__constant float2 sph_coord = (float2)(M_PI, 2.f * M_PI);

bool ray_sphere_intersection(Hit* hit, Ray ray, Metadata m_sphere, __global float* prims);

bool intersect(Hit* hit, Ray ray, __global Metadata* meta_prims, __global float* prims,
				int nb_prims);

Color lookup(image2d_t env, Ray ray);

//Color radiance(image2d_t env, Ray ray, __global Metadata* meta_prims, __global float* prims,
//			   uint env_w, uint nb_prims, RNG* rng, __global const Distribution1D* pConditionalV,
//			   Distribution1D pMarginal, __global const float* cdfConditionalV,
//			   __global const float* cdfMarginal, __global const float* fun2D,
//			   __global const float* fun1D);


#endif /* RAY_CASTER_H_ */
