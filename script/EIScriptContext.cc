#include "EIScriptContext.h"


void EIScript::EIScriptContext::clear_script()
{
}


void EIScript::EIScriptContext::addFunction(FunctionDeclaration* function)
{
    functions.insert(std::make_pair(function->id.name, function));
}
