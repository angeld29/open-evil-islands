#include <iostream>
#include <fstream>

#include "EIScriptScanner.h"
#include "EIScriptParser.h"
#include "EIScriptDriver.h"
#include "EIScriptContext.h"

// Entry Point
int main(int argc, char* argv[])
{
    EIScript::EIScriptContext* script_context = new EIScript::EIScriptContext();
    EIScript::Driver driver(script_context, false, true, false);

    for(int ai = 1; ai < argc; ++ai) {
        if(argv[ai] == std::string("-p")) {
            driver.trace_parsing = true;
        } else if(argv[ai] == std::string("-sp")) {
            driver.standard_trace_parsing = true;
        } else if(argv[ai] == std::string("-s")) {
            driver.trace_scanning = true;
        } else {
            // read a script file

            std::fstream infile(argv[ai]);
            if(!infile.good()) {
                std::cerr << "Could not open file: " << argv[ai] << std::endl;
                return 0;
            }
            driver.parse_stream(infile, argv[ai]);
        }
    }

    return 0;
}