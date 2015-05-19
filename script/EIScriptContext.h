
#pragma once

#include "eiscript_base.h"
#include "boost/unordered_map.hpp"
#include <string>

namespace EIScript
{

    class EIScriptContext
    {
    public:
        void clear_script();

        void addFunction(FunctionDeclaration* function);

    protected:
        EIScriptContext* parent;
        boost::unordered_map<std::string, FunctionDeclaration*> functions;
        boost::unordered_map<std::string, FunctionDeclaration*> scripts;
        boost::unordered_map<std::string, VariableDeclaration*> variables;
        FunctionDeclaration* worldscript;
    };

}
