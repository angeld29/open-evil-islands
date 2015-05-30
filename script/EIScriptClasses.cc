#include "EIScriptClasses.h"
#include "EIScriptContext.h"

namespace EIScript
{
    Expression* Assignment::resolve(EIScriptContext* context)
    {
        context->getVariable(lhs).setValue(rhs);
        return nullptr;
    }
    
    Expression* FunctionCall::resolve(EIScriptContext* context)
    {
        return context->call(functionName, arguments);
    }

}
