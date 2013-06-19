/*
 * host.cpp
 *
 *  Created on: 18 juin 2013
 *      Author: poupine
 */

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include "host.h"

using namespace std;

LoggerPtr logger(Logger::getLogger(__FILE__));

Host& Host::instance() {
	static Host _instance;
	return _instance;
}

void Host::buildKernels(string path) {
	cl_int error;

	DIR* dir;
	struct dirent *ent;
	cl::Program::Sources sources;

	dir = opendir(path.c_str());

	if (NULL != dir) {
		while (ent = readdir(dir)) {
			string fname = path + "/" + string(ent->d_name);
			if (string::npos != fname.find(".cl")) {
				ifstream f(fname.c_str());
				string source;
				f.seekg(0, ios::end);
				source.resize(f.tellg());
				f.seekg(0, ios::beg);
				f.read(&source[0], source.size());
				f.close();
				sources.push_back(make_pair(source.c_str(), source.size()));
			}
		}
		closedir(dir);
	}
	else {
		string err("Unknown path: " + path);
		throw cl::Error(CL_INVALID_VALUE, err.c_str());
	}

	_prog = new cl::Program(*_context, sources, &error);
	_prog->build(_devices, NULL, NULL, &error);

	LOG(logger, DEBUG, buildLog());
}

string Host::platforms() {

	cl::STRING_CLASS name, vendor, version, profile, ext;
	ostringstream info;

	for (VECTOR_CLASS<cl::Platform>::iterator it = _platforms.begin();
		it != _platforms.end(); ++it) {
		it->getInfo(CL_PLATFORM_NAME, &name);
		it->getInfo(CL_PLATFORM_VENDOR, &vendor);
		it->getInfo(CL_PLATFORM_VERSION, &version);
		it->getInfo(CL_PLATFORM_PROFILE, &profile);
		it->getInfo(CL_PLATFORM_EXTENSIONS, &ext);
		info << "Number of platforms found: " << _platforms.size() << endl
			 << "Platform Name: " << name << endl
			 << "Vendor: " << vendor << endl
			 << "OpenCL Version supported: " << version << endl
			 << "Standard: " << profile << endl
			 << "Extensions supported: " << ext << endl;
	}

	return info.str();

}

string Host::device() {
	cl::STRING_CLASS name, vendor, version, driver, ext;
	cl_uint nb_units;
	cl_device_type type;
	ostringstream info;

	_devices[d_index].getInfo(CL_DEVICE_TYPE, &type);
	_devices[d_index].getInfo(CL_DEVICE_NAME, &name);
	_devices[d_index].getInfo(CL_DEVICE_VENDOR, &vendor);
	_devices[d_index].getInfo(CL_DEVICE_VERSION, &version);
	_devices[d_index].getInfo(CL_DRIVER_VERSION, &driver);
	_devices[d_index].getInfo(CL_DEVICE_EXTENSIONS, &ext);
	_devices[d_index].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &nb_units);

	info << "Device: " << (CL_DEVICE_TYPE_CPU == type ? "CPU" : "GPU") << endl
		 << "Device Name: " << name << endl
		 << "Vendor Name: " << vendor << endl
		 << "OpenCL Version supported: " << version << endl
		 << "Current Driver Version: " << driver << endl
		 << "Number of Parallel Compute Units: " << nb_units << endl
		 << "Extensions supported: " << ext << endl;

	return info.str();

}

string Host::buildLog() {
	cl::STRING_CLASS options, blog, status;
	ostringstream info;

	_prog->getBuildInfo(_devices[d_index], CL_PROGRAM_BUILD_OPTIONS, &options);
	_prog->getBuildInfo(_devices[d_index], CL_PROGRAM_BUILD_LOG, &blog);
	_prog->getBuildInfo(_devices[d_index], CL_PROGRAM_BUILD_STATUS, &status);

	info << "Building kernels" << endl
		 << "Build Options: " << options << endl
		 << "Build Status: " << status << endl
		 << endl << "Build Log: " << blog << endl;

	return info.str();
}

// Private methods

Host::Host() : p_index(0) {
	// List of the devices available on the default platform
	VECTOR_CLASS<cl::Device> devices;
	try {
		cl::Platform::get(&_platforms);
		LOG(logger, INFO, platforms());
		_platforms[p_index].getDevices(CL_DEVICE_TYPE_ALL, &_devices);
		check_gpu();
		LOG(logger, INFO, device());
		_context = new cl::Context(_devices);
	}
	catch (cl::Error e) {
		LOG(logger, ERROR, e.what());
		LOG(logger, ERROR, e.err());
	}
}

void Host::check_gpu() {

	cl_device_type t;
	cl::STRING_CLASS ext;

	if (0 != _devices.size()) {
		d_index = 0;
	}
	else {
		throw cl::Error(CL_DEVICE_NOT_FOUND, "No compatible device found.");
	}

	for (uint32_t i = 0; i < _devices.size(); ++i) {
		_devices[i].getInfo(CL_DEVICE_TYPE, &t);
		if (CL_DEVICE_TYPE_GPU == t) {
			d_index = i;
			LOG(logger, INFO, "GPU found");
		}
	}

}