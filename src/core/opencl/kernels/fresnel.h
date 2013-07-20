/*
 * fresnel.h
 *
 *  Created on: 5 Jul 2013
 *      Author: jmr12
 */

#ifndef FRESNEL_H_
#define FRESNEL_H_

#include "GPU.h"

#define ETA_VACUUM 1.0
#define ETA_GLASS 1.5
#define Kr 0.99

inline float3 reflection(Ray ray, float3 n) {
	return normalize((ray.direction - 2 * dot(ray.direction, n) * n));
}

inline float3 refraction(Color* reflectance, Ray ray, float3 n, RNG* rng) {
	//rng->c.v[0]++;
	threefry4x32_ctr_t r = threefry4x32(rng->c, rng->k);

	float3 n_real = dot(ray.direction, n) < 0 ? n : -n;
	bool outside = dot(n, n_real) > 0;
	float cosi = dot(ray.direction, n_real);
	float eta1Overeta2 = outside ? ETA_VACUUM/ETA_GLASS : ETA_GLASS/ETA_VACUUM;
	float cos2t = 1 - eta1Overeta2 * eta1Overeta2 * (1 - cosi * cosi);

	if (cos2t < 0) { // Total Internal Reflection
		return reflection(ray, n_real);
	}

	float3 transdir = normalize(eta1Overeta2 * ray.direction - n_real *
			(eta1Overeta2 * cosi + sqrt(cos2t)));

	float num = ETA_GLASS-ETA_VACUUM;
	float denom = ETA_VACUUM + ETA_GLASS;
	float R0 = num * num/(denom * denom);
	float cost = 1-(outside?-cosi:dot(n, transdir));
	float Re = R0 + (1-R0) * cost * cost * cost * cost * cost; // Schlick's approximation
	float Tr = 1.f-Re;

	float rand = u01_open_open_32_24(r.v[1]);

	if (rand <= 0.5) {
		*reflectance *= 2.f * Re;
		return reflection(ray, n_real);
	}
	else {
		*reflectance *= 2.f * Tr;
		return transdir;
	}
}

#endif /* FRESNEL_H_ */
