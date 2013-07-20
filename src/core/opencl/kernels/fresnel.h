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
	rng->c.v[0]++;
	threefry4x32_ctr_t r = threefry4x32(rng->c, rng->k);

	float3 n_real = dot(ray.direction, n) < 0 ? n : -n;
	bool outside = dot(n, n_real) > 0;
	float cosi = dot(ray.direction, n_real);
	float eta1Overeta2 = outside ? ETA_VACUUM/ETA_GLASS : ETA_GLASS/ETA_VACUUM;
	float cos2t = 1 - eta1Overeta2 * eta1Overeta2 * (1 - cosi * cosi);

	if (cos2t < 0) { // Total Internal Reflection
		return reflection(ray, n_real);
	}

	float3 transdir = normalize(eta1Overeta2 * ray.direction - n_real * (eta1Overeta2 * cosi + sqrt(cos2t)));

	float a= ETA_GLASS-ETA_VACUUM, b=ETA_VACUUM+ETA_GLASS, R0=a*a/(b*b), c = 1-(outside?-cosi:dot(n, transdir));
	float Re = R0 + (1-R0) * c * c * c * c * c; //Rpercent(ray, n, nc, nt);
	float Tr = 1.f-Re, P = 0.25 + 0.5 * Re, Rp = Re/P, Tp = Tr/(1-P);

	if (u01_open_open_32_24(r.v[0]) < 0.5f) {
		*reflectance *= Rp;
		return reflection(ray, n_real);
	}
	else {
		*reflectance *= Tp;
		return transdir;
	}
}

static inline float Rs(Ray ray, float3 n, float eta1, float eta2) {
	float cost = dot(ray.direction, n);
	float sint = sqrt(1 - cost * cost);
	float val = eta1/eta2 * sint;
	return pow((eta1 * cost - eta2 * sqrt(1 - val * val)) / (eta1 * cost + eta2 * sqrt(1 - val * val)), 2);
}

static inline float Rp(Ray ray, float3 n, float eta1, float eta2) {
	float cost = dot(ray.direction, n);
	float sint = sqrt(1 - cost * cost);
	float val = eta1/eta2 * sint;
	return pow((eta2 * sqrt(1 - val * val) - eta1 * cost) / (eta1 * cost + eta2 * sqrt(1 - val * val)), 2);
}

float Rpercent(Ray ray, float3 n, float eta1, float eta2);

#endif /* FRESNEL_H_ */
