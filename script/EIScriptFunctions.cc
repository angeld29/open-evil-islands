#include "EIScriptFunctions.h"
#include "exception.hh"

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

    EIScript::Type EIScriptFunctions::getFunctionType(std::string* function_name)
    {
        func* f = functions[*function_name];
        if(!f) {
            throw EIScript::Exception::InvalidAction("Function " + *function_name + " is not defined", "EIScriptFunctions");
        } else {
            return f->type;
        }
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
