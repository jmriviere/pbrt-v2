/*
 * oclcheck.cpp
 *
 *  Created on: 18 juin 2013
 *      Author: poupine
 */

#include <iostream>
#include <string>

#include "host.h"
#include "util.h"
#include "log4cxx/basicconfigurator.h"

using namespace std;

int main(int argc, char* argv[]) {
    BasicConfigurator::configure();

    cl_int t;

    string name;

    int i[4] = {4};

    cl::Event e;

    std::cout << Host::instance().platforms() << std::endl;
    Host::instance().buildKernels(KERNEL_PATH);
    cl::Kernel k = Host::instance().retrieveKernel("test");
    cl::Buffer b(*(Host::instance()._context), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), NULL);
    k.setArg(0, sizeof(int) * 4, &i);
    k.getInfo(CL_KERNEL_NUM_ARGS, &t);

    cout << t << endl;

    t = Host::instance()._queue->enqueueTask(k, NULL, &e);

    if (t == CL_INVALID_KERNEL_ARGS) {
    	cout << "prout" << endl;
    }

    e.wait();
    t = Host::instance()._queue->enqueueReadBuffer(b, CL_TRUE, 0, sizeof(int), &i);
    cout << i[0] << endl;
    return 0;

}
