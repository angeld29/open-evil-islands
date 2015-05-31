#ifndef EI_SCRIPT_FUNCTIONS
#define EI_SCRIPT_FUNCTIONS

#include <string>
#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"
#include "function.hh"

namespace cursedearth
{

    class EIScriptFunctions
    {
        typedef typename EIScript::Function::AbstractFunction func;
    public:
        EIScriptFunctions();
        ~EIScriptFunctions();

        bool functionDefined(std::string* function_name);
        EIScript::Type getFunctionType(std::string* function_name);
        void dumpFunctions(std::ostream& str);

    protected:
        boost::unordered_map<std::string, func*> functions;

        void initFunctions();
    };

}

#endif // EI_SCRIPT_FUNCTIONS
