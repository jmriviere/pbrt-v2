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

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

#define KERNEL_PATH "/home/poupine/Thesis/pbrt-v2/src/core/opencl/kernels"

#ifdef DEBUG
#define LOG(log,lvl,msg) LOG4CXX_##lvl(log,msg)
#else
#define LOG(log,lvl,msg)
#endif



#endif /* UTIL_H_ */
