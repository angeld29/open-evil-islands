
#pragma once

#include <string>
#include <boost/unordered_map.hpp>
#include "AIDirector.h"
#include "EIScriptClassesBase.h"
#include "EIScriptClasses.h"

namespace EIScript
{

    class EIScriptContext
    {
    public:
        EIScriptContext(cursedearth::AIDirector* ai_director) {
            this->parent = nullptr;
            this->ai_director = ai_director;
        }

        EIScriptContext(EIScriptContext* parent, cursedearth::AIDirector* ai_director)
            : EIScriptContext(ai_director) {
            this->parent = parent;
            scripts.insert(parent->scripts.cbegin(), parent->scripts.cend());
            variables.insert(parent->variables.cbegin(), parent->variables.cend());
        }

        EIScriptContext* getParentContext();
        EIScriptContext* extendedContext(VariableList* new_vars);

        void clear_script();

        void addScript(ScriptDeclaration* script);
        void addVariable(VariableDeclaration* variable);

        bool functionDefined(Identifier* ident);
        bool scriptDefined(Identifier* ident);
        bool variableDefined(Identifier* ident);

        FunctionDeclaration* getFunction(Identifier* ident);
        ScriptDeclaration* getScript(Identifier* ident);
        VariableDeclaration* getVariable(Identifier* ident);

        void setWorldscript(ScriptDeclaration* worldscript) {
            this->worldscript = worldscript;
        }

        void dumpFunctions(std::ostream& str);
        void dumpScripts(std::ostream& str);
        void dumpVariables(std::ostream& str);

    protected:
        EIScriptContext* parent;
        cursedearth::AIDirector* ai_director;
        boost::unordered_map<std::string, ScriptDeclaration*> scripts;
        boost::unordered_map<std::string, VariableDeclaration*> variables;
        ScriptDeclaration* worldscript;
    };
}
