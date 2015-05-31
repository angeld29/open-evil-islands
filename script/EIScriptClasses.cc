#include "EIScriptClasses.h"
#include "EIScriptContext.h"

namespace EIScript
{
    Expression* Assignment::resolve(EIScriptContext* context)
    {
        context->getVariable(lhs)->setValue(rhs);
        return nullptr;
    }

    Expression* VariableAccess::resolve(EIScriptContext* context)
    {
        return context->getVariable(id)->getValue();
    }

    Expression* FunctionCall::resolve(EIScriptContext* context)
    {
        return context->call(functionName->name, arguments);
    }

    Expression* ScriptCall::resolve(EIScriptContext* context)
    {
        context->callScript(functionName->name, arguments);
        return nullptr;
    }

}
