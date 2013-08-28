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

#define KERNEL_PATH "./src/core/opencl/kernels"

#ifdef DEBUG
#define LOG(log,lvl,msg) LOG4CXX_##lvl(log,msg)
#else
#define LOG(log,lvl,msg)
#endif

enum OCLType {
	sphere = 0,
	trianglemesh = 1,
	light = 2
};

enum OCLMaterial {
	DIFF = 0,
	SPEC = 1,
	REFR = 2,
	ROUGH = 3
};

#pragma pack(push, 1)

// Describes how to access data about light and primitives
typedef struct s_metadata {
	float toWorld[16];
	float fromWorld[16];
	OCLType type;
	uint32_t offset;
	union {
		uint32_t dim[2]; // Light dimensions
		OCLMaterial mat; // Object material
	};
} Metadata;

typedef struct s_camera {
	float r2c[16];
	float c2w[16];
} OCLCamera;

#pragma pack(pop)

typedef struct s_sphere {
	float radius;
} OCLSphere;

typedef struct s_d1d {
	uint32_t offset;
	float integral;
	uint32_t count;
} OCLDistribution1D;

typedef struct s_d2d {
	OCLDistribution1D* pConditionalV;
	OCLDistribution1D* pMarginal;
} OCLDistribution2D;

#endif /* UTIL_H_ */
