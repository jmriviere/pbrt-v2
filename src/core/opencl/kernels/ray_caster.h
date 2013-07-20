/*
 * ray_caster.h
 *
 *  Created on: 5 Jul 2013
 *      Author: jmr12
 */

#ifndef RAY_CASTER_H_
#define RAY_CASTER_H_

#include "Random123/threefry.h"
#include "Random123/u01.h"

bool ray_sphere_intersection(Hit* hit, Ray ray, Metadata m_sphere, __global float* prims);
bool intersect(Hit* hit, Ray ray, __global Metadata* meta_prims, __global float* prims, int nb_prims);
Color lookup(image2d_t env, Ray ray);
Color radiance(image2d_t env, Ray ray, __global Metadata* meta_prims, __global float* prims,
	       int nb_prims, RNG* rng);


#endif /* RAY_CASTER_H_ */
