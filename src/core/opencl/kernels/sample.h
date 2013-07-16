/*
 * sample.h
 *
 *  Created on: 15 Jul 2013
 *      Author: jmr12
 */

#ifndef SAMPLE_H_
#define SAMPLE_H_

#include "Random123/threefry.h"
#include "Random123/u01.h"

void stratifiedSample2D(float samp[4], bool jitter) {
	float dx = 0.5f, dy = 0.5f;
	for (uint y = 0; y < 2; y++) {
		for (uint x = 0; x < 2; x++) {
			float jx = 0.5f;
			float jy = 0.5f;
			*samp++ = (x + jx) * dx;
			*samp++ = (y + jy) * dy;
		}
	}
}



#endif /* SAMPLE_H_ */
