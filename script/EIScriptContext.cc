#include "EIScriptContext.h"

void EIScript::EIScriptContext::clear_script()
{
}

void EIScript::EIScriptContext::addFunction(FunctionDeclaration* function)
{
    functions.insert(std::make_pair(*function->id->name, function));
    // functions.emplace(function->id->name, function);
}

void EIScript::EIScriptContext::addScript(FunctionDeclaration* script)
{
    scripts.insert(std::make_pair(*script->id->name, script));
    // scripts.emplace(script->id->name, script);
}

void EIScript::EIScriptContext::addVariable(VariableDeclaration* variable)
{
    variables.insert(std::make_pair(*variable->id->name, variable));
    // variables.emplace(variable->id->name, variable);
}


bool EIScript::EIScriptContext::functionDefined(Identifier* ident)
{
    return functions.find(*(ident->name)) != functions.end();
}

bool EIScript::EIScriptContext::scriptDefined(Identifier* ident)
{
    return scripts.find(*(ident->name)) != scripts.end();
}

bool EIScript::EIScriptContext::variableDefined(Identifier* ident)
{
    return variables.find(*(ident->name)) != variables.end();
}

void EIScript::EIScriptContext::dumpFunctions(std::ostream& str)
{
    for(auto& pair : functions) {
        str << "Function " << pair.first << " : " << pair.second << std::endl;
    }
}

void EIScript::EIScriptContext::dumpScripts(std::ostream& str)
{
    for(auto& pair : scripts) {
        str << "Script " << pair.first << " : " << pair.second << std::endl;
    }
}

void EIScript::EIScriptContext::dumpVariables(std::ostream& str)
{
    for(auto& pair : variables) {
        str << "Variable " << pair.first << " : " << pair.second << std::endl;
    }
}


