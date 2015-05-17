
#pragma once

#include "eiscript_base.h"
#include "boost/unordered_map.hpp"
#include <string> 

class EIScriptContext {
public:
    void addFunction(FunctionDeclaration* function){
        functions.insert(std::make_pair(function->id.name, function));
    }

protected:
    EIScriptContext* parent;
    boost::unordered_map<std::string, FunctionDeclaration*> functions;
    boost::unordered_map<std::string, FunctionDeclaration*> scripts;
    boost::unordered_map<std::string, VariableDeclaration*> variables;
    FunctionDeclaration* worldscript;

};