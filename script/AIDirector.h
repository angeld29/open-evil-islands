#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <string>
#include "EIScriptFunctions.h"
#include "EIScriptClassesBase.h"

namespace cursedearth
{

    template<class ScriptFunctions = EIScriptFunctions>
    class AIDirector
    {
    public:
        AIDirector() {
            functions_impl = new ScriptFunctions();
        }
        ~AIDirector() {
            delete functions_impl;
        }

        inline EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList* arguments) {
            return nullptr;
        }
        
        inline bool functionDefined(std::string* function_name) {
            return functions_impl->functionDefined(function_name);
        }

        inline void dumpFunctions(std::ostream& str) {
            functions_impl->dumpFunctions(str);
        }

    protected:
        ScriptFunctions* functions_impl;
    };

}

#endif // AI_DIRECTOR
