#ifndef EI_SCRIPT_EXECUTOR
#define EI_SCRIPT_EXECUTOR

#include <queue>
#include <tuple>

#include "EIScriptContext.h"
#include "EIScriptFunctions.h"
#include "exception.hh"
#include "log.h"

namespace EIScript
{

    class EIScriptExecutorImpl;

    class EIScriptExecutor
    {
    public:
        EIScriptExecutor(EIScriptContext* script_context, EIScriptFunctionsBase* functions_impl);
        ~EIScriptExecutor();

        void advance() ;
        void callScript(Identifier* function_name, ExpressionList* arguments);
        Expression* callFunction(std::string* function_name, ExpressionList* arguments);
        void setVerboseExecution(bool verbose);
        bool getVerboseExecution();

        VariableDeclaration* getVariable(const Identifier* ident) {
            return script_context->getVariable(ident);
        }

        bool functionDefined(Identifier* function_name) {
            return functions_impl->functionDefined(function_name->name);
        }

        bool functionDefined(std::string* function_name) {
            return functions_impl->functionDefined(function_name);
        }

        Type getFunctionType(std::string* function_name) {
            return functions_impl->getFunctionType(function_name);
        }

        void dumpFunctions(std::ostream& str) {
            functions_impl->dumpFunctions(str);
        }

    private:
        // all this decoupling thing may just turn out to be bullshit, we'll see. Should be easy to get rid of, if need be
        EIScriptExecutorImpl* executor_impl;
        EIScriptFunctionsBase* functions_impl;
        EIScriptContext* script_context;
    };

}

#endif // EI_SCRIPT_EXECUTOR
