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
	const cl::CommandQueue *_queue;
	const cl::Context* _context;

	~Host();

	static Host& instance();

	void buildKernels(std::string path);
	cl::Kernel retrieveKernel(cl::STRING_CLASS name);
	std::string platforms();
	std::string device();
	std::string buildLog(cl::Program prog);

private:
	Host();
	Host(Host const&);
	void operator=(Host const&);
	void check_gpu();

	uint32_t p_index;
	uint32_t d_index;
	VECTOR_CLASS<cl::Platform> _platforms;
	VECTOR_CLASS<cl::Device> _devices;
	std::vector<cl::Kernel> _kernels;

};


#endif /* HOST_H_ */
