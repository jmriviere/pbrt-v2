/*
 * fresnel.h
 *
 *  Created on: 5 Jul 2013
 *      Author: jmr12
 */

#ifndef FRESNEL_H_
#define FRESNEL_H_

#include "OCL.h"

#define ETA_VACUUM 1.0f
#define ETA_GLASS 1.5f
#define Kr 0.99f

inline float3 reflection(Ray ray, float3 n) {
	return normalize((ray.direction - 2 * dot(ray.direction, n) * n));
}

inline float3 refraction(Color* reflectance, Ray ray, float3 n, float rand) {

	float3 n_real = dot(ray.direction, n) < 0 ? n : -n;
	bool outside = dot(n, n_real) > 0;
	float cosi = dot(ray.direction, n_real);
	float eta1Overeta2 = outside ? ETA_VACUUM/ETA_GLASS : ETA_GLASS/ETA_VACUUM;
	float cos2t = 1 - eta1Overeta2 * eta1Overeta2 * (1 - cosi * cosi);

	if (cos2t < 0) { // Total Internal Reflection
	  return reflection(ray, n);
	}

	float3 transdir = normalize(eta1Overeta2 * ray.direction - n_real *
			(eta1Overeta2 * cosi + sqrt(cos2t)));

	float num = ETA_GLASS-ETA_VACUUM;
	float denom = ETA_VACUUM + ETA_GLASS;
	float R0 = num * num/(denom * denom);
	float3 halfway = (transdir + ray.direction);
	halfway /= dot(halfway, halfway);
	float cost = 1-dot(halfway, transdir);
	float Re = R0 + (1-R0) * cost * cost * cost * cost * cost; // Schlick's approximation
	float Tr = 1.f-Re, P = .25f + .75f * Re;

	if (rand <= P) {
		*reflectance *= Re/P;
		return reflection(ray, n_real);
	}
	else {
	  *reflectance *= Tr/(1-P);
		return transdir;
	}
}

#endif /* FRESNEL_H_ */
