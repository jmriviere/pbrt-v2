/*
 * host.h
 *
 *  Created on: 18 juin 2013
 *      Author: poupine
 */

#ifndef HOST_H_
#define HOST_H_

#ifdef MAC
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <boost/log/trivial.hpp>
#include "util.h"
#include "pbrt.h"

class Host {

	static void init();

private:
	Host();
	void info(VECTOR_CLASS<cl::Platform>);

	uint32_t p_index;
	VECTOR_CLASS<cl::Platform> _platforms;
	VECTOR_CLASS<cl::Device> _devices;

};


#endif /* HOST_H_ */
