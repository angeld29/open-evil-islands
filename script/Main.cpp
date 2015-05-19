#include <iostream>
#include <fstream>

#include "EIScriptScanner.h"
#include "EIScriptParser.h"
#include "EIScriptDriver.h"
#include "EIScriptContext.h"

// Entry Point
int main(int argc, char* argv[])
{
    EIScript::EIScriptContext script_context;
    EIScript::Driver driver(script_context);

    for(int ai = 1; ai < argc; ++ai) {
        if(argv[ai] == std::string("-p")) {
            driver.trace_parsing = true;
        } else if(argv[ai] == std::string("-s")) {
            driver.trace_scanning = true;
        } else {
            // read a script file

            std::fstream infile(argv[ai]);
            if(!infile.good()) {
                std::cerr << "Could not open file: " << argv[ai] << std::endl;
                return 0;
            }
            bool result = driver.parse_stream(infile, argv[ai]);
            //script_context.clear_script();
//            bool result = driver.parse_stream(infile, argv[ai]);
//            if(result) {
//                std::cout << "Expressions:" << std::endl;
//                for(unsigned int ei = 0; ei < script_context.expressions.size(); ++ei) {
//                    std::cout << "[" << ei << "]:" << std::endl;
//                    std::cout << "tree:" << std::endl;
//                    script_context.expressions[ei]->print(std::cout);
//                    std::cout << "evaluated: " << script_context.expressions[ei]->evaluate() << std::endl;
//                }
//            }
        }
    }

    return 0;
}