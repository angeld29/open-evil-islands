#include "EIScriptExecutor.h"

namespace EIScript
{

    /* DEBUG */

    std::ostream& operator << (std::ostream& os, std::nullptr_t)
    {
        return os << "nullptr";
    }

    /* DEBUG END */

    class EIScriptExecutorImpl
    {
    private:
        typedef std::tuple<ScriptDeclaration*, ExpressionList*> QueueEntry;
        std::queue<QueueEntry> script_queue;
        EIScriptExecutor* parent; // here -> parent -> callScript -> here - very dumb, look for a better way
        EIScriptContext* script_context;
        EIScriptFunctionsBase* functions_impl;
        bool verbose_execution;

        void push_context(ScriptDeclaration* scriptDeclaration, ExpressionList* arguments) {
            if(arguments) {
                ExpressionList resolved_arguments = resolveArguments(arguments);

                script_context = script_context->extendedContext(scriptDeclaration->getArguments());

                for(unsigned int i = 0; i < scriptDeclaration->getArguments()->size(); i++) {
                    if(resolved_arguments[i]) {
                        std::string* var_name = (*scriptDeclaration->getArguments())[i]->getName();
                        script_context->getVariable(var_name)->setValue(resolved_arguments[i]);
                    }
                    // TODO delete resolved expressions on pop
                }
            } else {
                script_context = script_context->extendedContext(scriptDeclaration->getArguments());
            }
        }

        void pop_context() {
            if(script_context->getParentContext() == nullptr) {
                std::cerr << "Tried to pop parent context." << std::endl;
            } else {
                EIScriptContext* previous = script_context;
                script_context = script_context->getParentContext();
                delete previous;
            }
        }

        ExpressionList resolveArguments(ExpressionList* arguments) {
            ExpressionList resolved_arguments(*arguments); //TODO is this even acceptable?
            auto& parent_val = parent;
            std::transform(
                resolved_arguments.begin(), resolved_arguments.end(), resolved_arguments.begin(),
            [parent_val](Expression* e) { if(e) return e->resolve(parent_val); else return e; }
            );
            return resolved_arguments;
        }

        void doExecuteScript(ScriptDeclaration* script) {
            Identifier* function_name = script->getId();
            for(auto block : *(script->getScriptBody())) {
                auto if_block = std::get<0>(*block);
                auto then_block = std::get<1>(*block);

                std::cout<<"IF in "<<*(function_name->name)<<std::endl;
                if(if_block) {
                    for(auto predicate : *(if_block)) {
                        if(predicate) {
                            std::cout<<predicate->resolve(parent)<<std::endl;
                        } else {
                            std::cout<<"nulltpr in if"<<std::endl;
                        }
                    }
                } else {
                    std::cout<<"empty"<<std::endl;
                }
                std::cout<<"THEN in "<<*(function_name->name)<<std::endl;
                if(then_block) {
                    for(auto expression : *(then_block)) {
                        if(expression) {
                            std::cout<<expression->resolve(parent)<<std::endl;
                        } else {
                            std::cout<<"nulltpr in then"<<std::endl;
                        }
                    }
                } else {
                    std::cout<<"empty (somehow)"<<std::endl;
                }
            }
        }

    public:
        EIScriptExecutorImpl(EIScriptExecutor* parent, EIScriptContext* script_context, EIScriptFunctionsBase* functions_impl)
            : parent(parent)
            , script_context(script_context)
            , functions_impl(functions_impl) { }

        void advance() {
            QueueEntry& entry = script_queue.front();
            executeScript(std::get<0>(entry), std::get<1>(entry));
            script_queue.pop();
        }

        // TODO FIXME XXX I could move this to EIScriptExecutor, but the parent is still required for expression resolution in callScript -> something better?
        inline Expression* callFunction(std::string* function_name, ExpressionList* arguments) {
            ExpressionList resolved_arguments = resolveArguments(arguments);
            return functions_impl->call(function_name, resolved_arguments);
        }

        void callScript(Identifier* function_name, ExpressionList* arguments) {
            ScriptDeclaration* script = script_context->getScript(function_name);
            if(!script) {
                if(function_name == script_context->getWorldscript()->getId()) { // possible issue with improper string comparison?
                    script = script_context->getWorldscript();
                } else {
                    throw Exception::InvalidAction("Script " + *function_name->name + " is not defined", "EIScriptExecutor->EIScriptContext");
                }
            }
            script_queue.emplace(script, arguments);
        }

        void executeScript(ScriptDeclaration* script, ExpressionList* arguments) {
            push_context(script, arguments);
            doExecuteScript(script);
            pop_context();
        }

        void setVerboseExecution(bool verbose) {
            verbose_execution = verbose;
        }

        inline bool getVerboseExecution() {
            return verbose_execution;
        }
    };

    EIScriptExecutor::EIScriptExecutor(EIScriptContext* script_context, EIScriptFunctionsBase* functions_impl)
        : executor_impl(new EIScriptExecutorImpl(this, script_context, functions_impl))
        , functions_impl(functions_impl)
        , script_context(script_context) { }

    EIScriptExecutor::~EIScriptExecutor()
    {
        delete executor_impl;
    }

    void EIScriptExecutor::advance()
    {
        executor_impl->advance();
    }

    void EIScriptExecutor::callScript(Identifier* function_name, ExpressionList* arguments)
    {
        executor_impl->callScript(function_name, arguments);
    }

    Expression* EIScriptExecutor::callFunction(std::string* function_name, ExpressionList* arguments)
    {
        return executor_impl->callFunction(function_name, arguments);
    }

    void EIScriptExecutor::setVerboseExecution(bool verbose)
    {
        executor_impl->setVerboseExecution(verbose);
    }

    bool EIScriptExecutor::getVerboseExecution()
    {
        return executor_impl->getVerboseExecution();
    }

}
