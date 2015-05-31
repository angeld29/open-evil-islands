
#pragma once

#include <string>
#include <boost/unordered_map.hpp>
#include "AIDirector.h"
#include "EIScriptClassesBase.h"
#include "exception.hh"

namespace EIScript
{
    class VariableDeclaration;
    class ScriptDeclaration;

    class EIScriptContext
    {
    public:
        EIScriptContext(cursedearth::AIDirector<>* ai_director)
            : parent(nullptr)
            , ai_director(ai_director) {
        }

        EIScriptContext(EIScriptContext* parent)
            : parent(parent)
            , ai_director(parent->ai_director) {
        }

        EIScriptContext* getParentContext();
        EIScriptContext* extendedContext(VariableList* new_vars);

        void clear_script();

        void addScript(ScriptDeclaration* script);
        void addGlobalVariable(VariableDeclaration* variable);
        void addLocalVariable(VariableDeclaration* variable);

        bool functionDefined(Identifier* ident);
        bool scriptDefined(Identifier* ident);
        bool variableDefined(Identifier* ident);

        Type getFunctionType(Identifier* ident); 
        ScriptDeclaration* getScript(Identifier* ident);
        VariableDeclaration* getVariable(const Identifier* ident);

        Expression* call(std::string* function_name, ExpressionList* arguments);
        void callScript(std::string* function_name, ExpressionList* arguments);

        void setWorldscript(ScriptDeclaration* worldscript) {
            this->worldscript = worldscript;
        }

        void dumpFunctions(std::ostream& str);
        void dumpScripts(std::ostream& str);
        void dumpVariables(std::ostream& str);

    protected:
        EIScriptContext* parent;
        cursedearth::AIDirector<>* ai_director;
        boost::unordered_map<std::string, ScriptDeclaration*> scripts;
        boost::unordered_map<std::string, VariableDeclaration*> globals;
        boost::unordered_map<std::string, VariableDeclaration*> locals;
        ScriptDeclaration* worldscript;
    };
}
