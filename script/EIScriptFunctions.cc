#include "EIScriptFunctions.h"

namespace cursedearth
{

    EIScriptFunctions::EIScriptFunctions()
    {
        initFunctions();
    }

    EIScriptFunctions::~EIScriptFunctions()
    {
    }

    bool EIScriptFunctions::functionDefined(std::string* function_name)
    {
        return functions.find(*function_name) != functions.end();
    }

    void EIScriptFunctions::dumpFunctions(std::ostream& str)
    {
        for(auto& pair : functions) {
            str << "Function " << pair.first << " : " << pair.second << std::endl;
        }
    }

    void EIScriptFunctions::initFunctions()
    {
        //functions[""]
    }

}
