/*
 * util.h
 *
 *  Created on: 18 juin 2013
 *      Author: poupine
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "log4cxx/basicconfigurator.h"

using namespace log4cxx;
using namespace log4cxx::xml;

#define KERNEL_PATH "/homes/jmr12/Thesis/pbrt-v2/src/core/opencl/kernels"

#ifdef DEBUG
#define LOG(log,lvl,msg) LOG4CXX_##lvl(log,msg)
#else
#define LOG(log,lvl,msg)
#endif

enum GPUType {
	sphere = 0,
	light = 1
};

enum GPUMaterial {
	DIFF = 0,
	SPEC = 1,
	REFR = 2
};

#pragma pack(push, 1)

typedef struct s_ray {
	float origin[3];
	float dummy;
	float direction[3];
	float dummy2;
} gpu_Ray;

// Describes how to access data about light and primitives
typedef struct s_metadata {
	float toWorld[16];
	float fromWorld[16];
	GPUType type;
	uint32_t offset;
	union {
		uint32_t dim[2]; // Light dimensions
		GPUMaterial mat; // Object material
	};
} Metadata;

typedef struct s_camera {
	float r2c[16];
	float c2w[16];
} GPUCamera;

#pragma pack(pop)

typedef struct s_sphere {
	float radius;
} GPUSphere;

#endif /* UTIL_H_ */
