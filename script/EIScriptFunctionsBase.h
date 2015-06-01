#ifndef EI_SCRIPT_FUNCTIONS_BASE
#define EI_SCRIPT_FUNCTIONS_BASE

#include <string>
#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"
#include "function.hh"

namespace EIScript{
    class EIScriptFunctions;
}

namespace cursedearth
{

    //TODO refactor, unfuck
    template<class ScriptFunctions = typename EIScript::EIScriptFunctions>
    class AIDirector;

    //TODO is this ok?
    template<class T> 
    class EIScriptFunctionsBase
    {
    public:
        EIScriptFunctionsBase(AIDirector<T>* ai_director)
            : ai_director(ai_director) {}
            
        virtual ~EIScriptFunctionsBase() {}

        virtual EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList* arguments) = 0;

        virtual bool functionDefined(std::string* function_name) = 0;
        virtual EIScript::Type getFunctionType(std::string* function_name) = 0;
        virtual void dumpFunctions(std::ostream& str) = 0;
    protected:
        AIDirector<T>* ai_director;
    };

}

#endif // EI_SCRIPT_FUNCTIONS_BASE
