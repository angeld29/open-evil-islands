#include <iostream>
#include <fstream>

#include "AIDirector.hpp"
#include "EIScriptScanner.hpp"
#include "EIScriptParser.hpp"
#include "EIScriptDriver.hpp"
#include "EIScriptContext.hpp"
#include "EIScriptFunctions.hpp"
#include "EIScriptExecutor.hpp"


int main(int argc, char* argv[])
{
    cursedearth::AIDirector* ai_director = new cursedearth::AIDirector();
    EIScript::EIScriptContext* script_context = new EIScript::EIScriptContext();
    EIScript::EIScriptFunctions* script_functions = new EIScript::EIScriptFunctions(ai_director);
    EIScript::EIScriptExecutor* script_executor = new EIScript::EIScriptExecutor(script_context, script_functions);
    EIScript::Driver driver(script_context, script_executor, false, true, false);

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

            // DEBUG
            std::cout<<std::endl<<*(script_context->getWorldscript()->getScriptBody())<<std::endl;
            script_executor->callScript(script_context->getWorldscript()->getId(), nullptr);
            while(true) {
                script_executor->advance();
            }
        }
    }

    return 0;
}
