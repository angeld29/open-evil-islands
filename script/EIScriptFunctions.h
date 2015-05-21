#ifndef EI_SCRIPT_FUNCTIONS
#define EI_SCRIPT_FUNCTIONS

#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"

namespace cursedearth
{

    class EIScriptFunctions
    {
    public:
        EIScriptFunctions();
        ~EIScriptFunctions();
        
        void addFunction(EIScript::FunctionDeclaration* function);
        bool functionDefined(std::string* function_name);
        EIScript::FunctionDeclaration* getFunction(std::string* function_name);
        
    protected:
        boost::unordered_map<std::string, FunctionDeclaration*> functions;
    };

}

#endif // EI_SCRIPT_FUNCTIONS
