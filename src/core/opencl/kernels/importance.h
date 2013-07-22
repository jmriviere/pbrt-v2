/*
 * importance.h
 *
 *  Created on: 22 Jul 2013
 *      Author: jmr12
 */

#ifndef IMPORTANCE_H_
#define IMPORTANCE_H_

typedef struct s_d1d {
	uint offset;
	float integral;
	uint count;
} Distribution1D;

__constant float4 weight = (float4)(0.212671f, 0.715160f, 0.072169f, 0.f);

void sampleContinuous()


#endif /* IMPORTANCE_H_ */
