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

#include <string>

#include "util.h"
#include "pbrt.h"

class Host {
public:
	static Host& instance();

	void buildKernels(std::string path);
	std::string platforms();
	std::string device();
	std::string buildLog();

private:
	Host();
	Host(Host const&);
	void operator=(Host const&);
	void check_gpu();

	uint32_t p_index;
	uint32_t d_index;
	VECTOR_CLASS<cl::Platform> _platforms;
	VECTOR_CLASS<cl::Device> _devices;
	cl::Context* _context;
	cl::Program* _prog;

};


#endif /* HOST_H_ */
