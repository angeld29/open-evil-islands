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
        ~AIDirector();
        
        EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList* arguments);
        void addFunction(EIScript::FunctionDeclaration* function);
        bool functionDefined(std::string* function_name);
        EIScript::FunctionDeclaration* getFunction(std::string* function_name);
        void dumpFunctions(std::ostream& str);
        
    protected:
        ScriptFunctions* functions_impl;
    };

}

#endif // AI_DIRECTOR
