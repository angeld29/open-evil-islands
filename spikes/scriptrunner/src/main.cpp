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

    for(int ai = 1; ai < argc; ++ai) {
        if(argv[ai] == std::string("-p")) {
            ai_director->setMyTraceParsing(true);
        } else if(argv[ai] == std::string("-sp")) {
            ai_director->setTraceParsing(true);
        } else if(argv[ai] == std::string("-s")) {
            ai_director->setTraceScanning(true);
        } else {
            // read a script file

            std::fstream infile(argv[ai]);
            if(!infile.good()) {
                std::cerr << "Could not open file: " << argv[ai] << std::endl;
                return 0;
            }
            ai_director->parseStream(infile, argv[ai]);

            ai_director->startExecution();
            while(true) {
                ai_director->advance(0);
            }
        }
    }

    return 0;
}
