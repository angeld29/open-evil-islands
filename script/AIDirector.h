#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <string>
#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"
#include "EIScriptFunctionsBase.h"
#include "EIScriptFunctions.h"
#include "ScriptAccessible.hh"

namespace cursedearth
{

    template<class ScriptFunctions> // = typename EIScript::EIScriptFunctions>
    class AIDirector : public EIScript::ScriptAccessible
    {
    public:
        AIDirector() {
            functions_impl = new ScriptFunctions(this);
        }
        ~AIDirector() {
            delete functions_impl;
        }

        inline EIScript::Expression* call(std::string* function_name, EIScript::ExpressionList& arguments) {
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
        
        void GSSetVar(double player, std::string* variable, double value);
        void GSSetVarMax(double player, std::string* variable, double value);
        double GSGetVar(double player, std::string* variable);
        void GSDelVar(double player, std::string* variable);

    protected:
        EIScriptFunctionsBase<ScriptFunctions>* functions_impl;
        boost::unordered_map<std::string, double> gs_vars; // Why gs_vars? GSSetVar? What is GS?
    };

}

#endif // AI_DIRECTOR
