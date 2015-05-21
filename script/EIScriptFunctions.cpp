#include "EIScriptFunctions.h"

namespace cursedearth
{

    EIScriptFunctions::EIScriptFunctions()
    {
    }

    EIScriptFunctions::~EIScriptFunctions()
    {
    }

    void EIScriptFunctions::addFunction(FunctionDeclaration* function)
    {
        functions.insert(std::make_pair(*function->id->name, function));
    }

    bool EIScriptFunctions::functionDefined(std::string* function_name)
    {
        return functions.find(*function_name) != functions.end();
    }

    FunctionDeclaration* EIScriptFunctions::getFunction(std::string* function_name)
    {
        return functions[*function_name];
    }

    void EIScriptFunctions::dumpFunctions(std::ostream& str)
    {
        for(auto& pair : functions) {
            str << "Function " << pair.first << " : " << pair.second << std::endl;
        }
    }


}
