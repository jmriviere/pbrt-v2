/*
 * oclcheck.cpp
 *
 *  Created on: 18 juin 2013
 *      Author: poupine
 */

#include <iostream>

#include "host.h"
#include "util.h"
#include "log4cxx/basicconfigurator.h"

int main(int argc, char* argv[]) {
    BasicConfigurator::configure();

    std::cout << Host::instance().platforms() << std::endl;
    Host::instance().buildKernels(KERNEL_PATH);
    return 0;

}
