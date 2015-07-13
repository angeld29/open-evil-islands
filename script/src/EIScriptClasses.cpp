#include "EIScriptClasses.hpp"
#include "EIScriptExecutor.hpp"

namespace EIScript
{

    Expression* Assignment::resolve(EIScriptExecutor* executor)
    {
        if(rhs) {
            executor->getVariable(lhs)->setValue(rhs->resolve(executor));
        } else {
            executor->getVariable(lhs)->setValue(nullptr);
        }
        return nullptr;
    }

    Expression* VariableAccess::resolve(EIScriptExecutor* executor)
    {
        return executor->getVariable(id)->getValue();
    }

    Expression* FunctionCall::resolve(EIScriptExecutor* executor)
    {
        Expression* result = executor->callFunction(functionName->name, arguments);
        if(executor->getVerboseExecution()) {
            std::cout<<"Function "<<*(functionName->name)<<" result: ";
            if(result) {
                std::cout<<*result<<std::endl;
            } else {
                std::cout<<"nullptr"<<std::endl;
            }
        }
        return result;
    }

    Expression* ScriptCall::resolve(EIScriptExecutor* executor)
    {
        executor->callScript(functionName, arguments);
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
