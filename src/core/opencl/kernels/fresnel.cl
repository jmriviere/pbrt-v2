/*
 * fresnel.cl
 *
 *  Created on: 5 Jul 2013
 *      Author: jmr12
 */


#include "fresnel.h"

float Rpercent(Ray ray, float3 n, float eta1, float eta2) {
	return 1/2. * (Rs(ray, n, eta1, eta2) + Rp(ray, n, eta1, eta2));
}

