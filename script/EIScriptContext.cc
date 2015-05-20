#include "EIScriptContext.h"

namespace EIScript
{
    EIScriptContext* EIScriptContext::getParentContext()
    {
        return parent;
    }

    EIScriptContext* EIScriptContext::extendedContext(VariableList* new_vars)
    {
        EIScriptContext* extended = new EIScriptContext(this);
        if(new_vars) {
            for(auto& var : *new_vars) {
                extended->addVariable(var);
            }
        }
        return extended;
    }


    void EIScriptContext::clear_script()
    {
    }

    void EIScriptContext::addFunction(FunctionDeclaration* function)
    {
        functions.insert(std::make_pair(*function->id->name, function));
        // functions.emplace(function->id->name, function);
    }

    void EIScriptContext::addScript(FunctionDeclaration* script)
    {
        scripts.insert(std::make_pair(*script->id->name, script));
        // scripts.emplace(script->id->name, script);
    }

    void EIScriptContext::addVariable(VariableDeclaration* variable)
    {
        variables.insert(std::make_pair(*variable->id->name, variable));
        // variables.emplace(variable->id->name, variable);
    }


    bool EIScriptContext::functionDefined(Identifier* ident)
    {
        return functions.find(*(ident->name)) != functions.end();
    }

    bool EIScriptContext::scriptDefined(Identifier* ident)
    {
        return scripts.find(*(ident->name)) != scripts.end();
    }

    bool EIScriptContext::variableDefined(Identifier* ident)
    {
        return variables.find(*(ident->name)) != variables.end();
    }

    FunctionDeclaration* EIScriptContext::getFunction(Identifier* ident)
    {
        return functions[*ident->name];
    }

    FunctionDeclaration* EIScriptContext::getScript(Identifier* ident)
    {
        return scripts[*ident->name];
    }

    VariableDeclaration* EIScriptContext::getVariable(Identifier* ident)
    {
        return variables[*ident->name];
    }


    void EIScriptContext::dumpFunctions(std::ostream& str)
    {
        for(auto& pair : functions) {
            str << "Function " << pair.first << " : " << pair.second << std::endl;
        }
    }

    void EIScriptContext::dumpScripts(std::ostream& str)
    {
        for(auto& pair : scripts) {
            str << "Script " << pair.first << " : " << pair.second << std::endl;
        }
    }

    void EIScriptContext::dumpVariables(std::ostream& str)
    {
        for(auto& pair : variables) {
            str << "Variable " << pair.first << " : " << pair.second << std::endl;
        }
    }
}
