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
        Expression* result = context->callFunction(functionName->name, arguments);
        std::cout<<"Function "<<*(functionName->name)<<" result: "<<*result;
        return result;
    }

    Expression* ScriptCall::resolve(EIScriptContext* context)
    {
        context->callScript(functionName, arguments);
        return nullptr;
    }

    void Assignment::log(std::ostream& out)
    {
        out<<"Assignment: "<<rhs;
    }

    void VariableAccess::log(std::ostream& out)
    {
        out<<"Variable access: "<<*id->name;
    }

    void FunctionCall::log(std::ostream& out)
    {
        out<<"Function call: "<<*functionName->name;
    }

    void ScriptCall::log(std::ostream& out)
    {
        out<<"Script call: "<<*functionName->name;
    }

}
