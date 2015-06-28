#ifndef EI_SCRIPT_FUNCTIONS_BASE
#define EI_SCRIPT_FUNCTIONS_BASE

#include <string>
#include <boost/unordered_map.hpp>

#include "EIScriptClassesBase.hpp"

namespace cursedearth {
    class AIDirector;
}

namespace EIScript
{

    class EIScriptFunctionsBase
    {
    protected:
        typedef typename cursedearth::AIDirector DirectorType;
        DirectorType* ai_director;

    public:
        EIScriptFunctionsBase(DirectorType* ai_director)
            : ai_director(ai_director) {}

        virtual ~EIScriptFunctionsBase() {}

        //TODO handle no args
        virtual EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList& arguments) = 0;

        virtual bool functionDefined(std::string* function_name) = 0;
        virtual EIScript::Type getFunctionType(std::string* function_name) = 0;
        virtual void dumpFunctions(std::ostream& str) = 0;

    };

}

#endif // EI_SCRIPT_FUNCTIONS_BASE
