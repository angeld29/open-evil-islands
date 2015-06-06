
#pragma once

#include <string>
#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"
#include "EIScriptClasses.h"
#include "exception.hh"
#include "util.h"
#include "log.h"

namespace EIScript
{
    class VariableDeclaration;
    class ScriptDeclaration;

    class EIScriptContext
    {
    public:
        EIScriptContext();
        EIScriptContext(EIScriptContext* parent);
        ~EIScriptContext();
        
        EIScriptContext* getParentContext();
        EIScriptContext* extendedContext(VariableList* new_vars);

        void clear_script();

        void addScript(ScriptDeclaration* script);
        void addGlobalVariable(VariableDeclaration* variable);
        void addLocalVariable(VariableDeclaration* variable);

        bool scriptDefined(Identifier* ident);
        bool variableDefined(Identifier* ident);

        ScriptDeclaration* getScript(Identifier* ident);
        ScriptDeclaration* getScript(std::string* script_name);
        VariableDeclaration* getVariable(const Identifier* ident);
        VariableDeclaration* getVariable(std::string* var_name);
        
        ScriptDeclaration* getWorldscript();
        void setWorldscript(ScriptDeclaration* worldscript);

        void dumpScripts(std::ostream& str);
        void dumpVariables(std::ostream& str);

    protected:
        EIScriptContext* parent;
        boost::unordered_map<std::string, ScriptDeclaration*, CaseInsensitiveHash, CaseInsensitiveEqual> scripts;
        boost::unordered_map<std::string, VariableDeclaration*, CaseInsensitiveHash, CaseInsensitiveEqual> globals;
        boost::unordered_map<std::string, VariableDeclaration*, CaseInsensitiveHash, CaseInsensitiveEqual> locals;
        ScriptDeclaration* worldscript;
    };
}
