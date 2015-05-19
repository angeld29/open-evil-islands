
#pragma once

#include "EIScriptClasses.h"
#include "boost/unordered_map.hpp"
#include <string>

namespace EIScript
{

class EIScriptContext
{
public:
    EIScriptContext()
    {
        this->parent = nullptr;
    }

    EIScriptContext(EIScriptContext* parent)
    {
        this->parent = parent;
        functions.insert(parent->functions.cbegin(), parent->functions.cend());
        scripts.insert(parent->scripts.cbegin(), parent->scripts.cend());
        variables.insert(parent->variables.cbegin(), parent->variables.cend());
    }
    
    void clear_script();

    void addFunction(FunctionDeclaration* function);
    void addScript(FunctionDeclaration* script);
    void addVariable(VariableDeclaration* variable);
    
    bool functionDefined(Identifier* ident);
    bool scriptDefined(Identifier* ident);
    bool variableDefined(Identifier* ident);

    void setWorldscript(FunctionDeclaration* worldscript)
    {
        this->worldscript = worldscript;
    }
    
    void dumpFunctions(std::ostream& str);
    void dumpScripts(std::ostream& str);
    void dumpVariables(std::ostream& str);
    
protected:
    EIScriptContext* parent;
    boost::unordered_map<std::string, FunctionDeclaration*> functions;
    boost::unordered_map<std::string, FunctionDeclaration*> scripts;
    boost::unordered_map<std::string, VariableDeclaration*> variables;
    FunctionDeclaration* worldscript;
};
}
