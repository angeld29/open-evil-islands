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

    //TODO this AIDirector usage does not allow to use a different functions implementations, rewrite
    class EIScriptFunctionsBase
    {
    public:
        EIScriptFunctionsBase(AIDirector<>* ai_director)
            : ai_director(ai_director) {}
            
        virtual ~EIScriptFunctionsBase() {}

        virtual EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList* arguments) = 0;

        virtual bool functionDefined(std::string* function_name) = 0;
        virtual EIScript::Type getFunctionType(std::string* function_name) = 0;
        virtual void dumpFunctions(std::ostream& str) = 0;
    protected:
        AIDirector<>* ai_director;
    };

}

#endif // EI_SCRIPT_FUNCTIONS_BASE
