/*
 * camera.h
 *
 *  Created on: 14 Jul 2013
 *      Author: jmr12
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "GPU.h"

typedef struct __attribute__ ((packed)) s_camera {

	Transformation r2c;
	Transformation c2w;

} GPUCamera;

Ray generate_ray(GPUCamera c, float2 raster) {
	Ray r;

	float3 Praster = (float3)(raster, 0);
	float3 dir = normalize(transform_point(Praster, c.r2c));

	r.origin = transform_point((0, 0, 0), c.c2w);
	r.direction = transform_vect(dir, c.c2w);

	return r;

}


#endif /* CAMERA_H_ */
