#include "EIScriptClasses.h"
#include "EIScriptContext.h"

namespace EIScript
{

    Expression* Assignment::resolve(ScriptContext* context)
    {
        context->getVariable(lhs)->setValue(rhs);
        return nullptr;
    }

    Expression* VariableAccess::resolve(ScriptContext* context)
    {
        return context->getVariable(id)->getValue();
    }

    Expression* FunctionCall::resolve(ScriptContext* context)
    {
        Expression* result = context->callFunction(functionName->name, arguments);
        if(context->getVerboseExecution()) {
            std::cout<<"Function "<<*(functionName->name)<<" result: ";
            if(result) {
                std::cout<<*result<<std::endl;
            } else {
                std::cout<<"nullptr"<<std::endl;
            }
        }
        return result;
    }

    Expression* ScriptCall::resolve(ScriptContext* context)
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
