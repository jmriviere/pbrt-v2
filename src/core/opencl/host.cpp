/*
 * host.cpp
 *
 *  Created on: 18 juin 2013
 *      Author: poupine
 */

#include "host.h"

Host::Host() : p_index(0) {
	try {
		cl::Platform::get(&_platforms);
		info(_platforms);
	}
	catch (cl::Error e) {
		Error(e.what());
		LOG(error) << e.what();
	}
}

void Host::info(VECTOR_CLASS<cl::Platform> platforms) {

	STRIN

	for (VECTOR_CLASS<cl::Platform>::iterator it = platforms.begin();
		it != platforms.end(); ++it) {
		it->getInfo()
	}

}
