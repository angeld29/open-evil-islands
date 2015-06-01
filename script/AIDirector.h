#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <string>
#include "EIScriptClassesBase.h"
#include "EIScriptFunctionsBase.h"
#include "EIScriptFunctions.h"

namespace cursedearth
{

    template<class ScriptFunctions> // = typename EIScript::EIScriptFunctions>
    class AIDirector
    {
    public:
        AIDirector() {
            functions_impl = new ScriptFunctions(this);
        }
        ~AIDirector() {
            delete functions_impl;
        }

        inline EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList* arguments) {
            return functions_impl->call(function_name, arguments);
        }
        
        inline bool functionDefined(std::string* function_name) {
            return functions_impl->functionDefined(function_name);
        }
        
        inline EIScript::Type getFunctionType(std::string* function_name){
            return functions_impl->getFunctionType(function_name);
        }

        inline void dumpFunctions(std::ostream& str) {
            functions_impl->dumpFunctions(str);
        }

    protected:
        EIScriptFunctionsBase* functions_impl;
    };

}

#endif // AI_DIRECTOR
