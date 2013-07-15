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

inline float3 refraction(Ray ray, float3 n, float eta1Overeta2) {
	float cosi = dot(ray.direction, n);
	float sin2t = eta1Overeta2 * eta1Overeta2 * (1 - cosi * cosi);
	return normalize(-eta1Overeta2 * ray.direction + (eta1Overeta2 * dot(ray.direction, n) + sqrt(1 - sin2t)) * n);
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
