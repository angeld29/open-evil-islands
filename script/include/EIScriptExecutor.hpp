#ifndef EI_SCRIPT_EXECUTOR
#define EI_SCRIPT_EXECUTOR

#include <queue>
#include <tuple>

#include "EIScriptContext.hpp"
#include "EIScriptFunctionsBase.hpp"
#include "scriptexception.hpp"

namespace EIScript
{

    class EIScriptExecutor
    {
    public:
        EIScriptExecutor(EIScriptContext* script_context, EIScriptFunctionsBase* functions_impl);
        ~EIScriptExecutor();

        void advance(float elapsed);

        //TODO handle no args
        void callScript(Identifier* function_name, ExpressionList* arguments);
        Expression* callFunction(std::string* function_name, ExpressionList* arguments);

        void setVerboseExecution(bool verbose);
        bool getVerboseExecution();

        bool queueIsEmpty();

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
        typedef std::tuple<ScriptDeclaration*, ExpressionList*> QueueEntry;
        std::queue<QueueEntry> script_queue;
        bool verbose_execution;
        EIScriptFunctionsBase* functions_impl;
        EIScriptContext* script_context;

        void executeScript(ScriptDeclaration* script, ExpressionList* arguments);
        void doExecuteScript(ScriptDeclaration* script);
        void push_context(ScriptDeclaration* scriptDeclaration, ExpressionList* arguments);
        void pop_context();
        ExpressionList resolveArguments(ExpressionList* arguments);
    };

}

#endif // EI_SCRIPT_EXECUTOR
