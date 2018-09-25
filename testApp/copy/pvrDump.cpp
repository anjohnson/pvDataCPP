/* pvrDump.cpp */
/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/* This program accepts a pvRequest string and prints out the pvStructure
 * that results from compiling that string.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <osiUnistd.h>
#include <pv/createRequest.h>
#include <pv/json.h>

using namespace epics::pvData;
using std::string;

const char *prog;

void usage(int status = 1, string error = string())
{
    if (!error.empty())
        std::cerr << error << std::endl;
    std::cerr << "Usage: " << prog << " [-j] [-1] '<request string>'" << std::endl;
    exit(status);
}

int main(int argc, const char **argv)
{
    int i = 0;
    prog = argv[i++];

    if (argc < 2) usage(2);

    bool opt_j = false;
    bool opt_1 = false;

    while (i < argc - 1) {
        string arg = argv[i++];
        if (arg == "-j") {
            opt_j = 1;
        }
        else if (arg == "-1") {
            opt_1 = true;
        }
        else {
            usage(2, string("Unknown option '") + arg + "'");
        }
    }

    CreateRequest::shared_pointer cr = CreateRequest::create();
    PVStructurePtr pvRequest = cr->createRequest(argv[i]);
    if (!pvRequest) {
        std::cerr << "Request failed: " << cr->getMessage() << std::endl;
        exit(1);
    }

    if (opt_j) {
        JSONPrintOptions opts;
        opts.multiLine = !opt_1;
        opts.indent = 0;
        opts.step = 2;

        printJSON(std::cout, pvRequest, opts);
        std::cout << std::endl;
    }
    else {
        std::cout << pvRequest << std::endl;
    }

    return 0;
}
