#include "EIScriptExecutor.hpp"
#include "logging.hpp"

namespace EIScript
{

    /* DEBUG */

    std::ostream& operator << (std::ostream& os, std::nullptr_t)
    {
        return os << "nullptr";
    }

    /* DEBUG END */

    EIScriptExecutor::EIScriptExecutor(EIScriptContext* script_context, EIScriptFunctionsBase* functions_impl)
        : functions_impl(functions_impl)
        , script_context(script_context) { }

    EIScriptExecutor::~EIScriptExecutor()
    {
    }

    void EIScriptExecutor::advance(float elapsed) {
        QueueEntry& entry = script_queue.front();
        executeScript(std::get<0>(entry), std::get<1>(entry));
        script_queue.pop();
    }

    void EIScriptExecutor::callScript(Identifier* function_name, ExpressionList* arguments) {
        ScriptDeclaration* script = script_context->getScript(function_name);
        if(!script) {
            if(function_name == script_context->getWorldscript()->getId()) { // possible issue with improper string comparison?
                script = script_context->getWorldscript();
            } else {
                throw Exception::InvalidAction("Script " + *function_name->name +
                                               " is not defined", "EIScriptExecutor->EIScriptContext");
            }
        }
        script_queue.emplace(script, arguments);
    }

    inline Expression* EIScriptExecutor::callFunction(std::string* function_name, ExpressionList* arguments) {
        ExpressionList resolved_arguments = resolveArguments(arguments);
        return functions_impl->call(function_name, resolved_arguments);
    }

    void EIScriptExecutor::setVerboseExecution(bool verbose) {
        verbose_execution = verbose;
    }

    inline bool EIScriptExecutor::getVerboseExecution() {
        return verbose_execution;
    }

    void EIScriptExecutor::executeScript(ScriptDeclaration* script, ExpressionList* arguments) {
        push_context(script, arguments);
        doExecuteScript(script);
        pop_context();
    }

    void EIScriptExecutor::doExecuteScript(ScriptDeclaration* script) {
        Identifier* function_name = script->getId();
        for(auto block : *(script->getScriptBody())) {
            auto if_block = std::get<0>(*block);
            auto then_block = std::get<1>(*block);

            std::cout<<"IF in "<<*(function_name->name)<<std::endl;
            if(if_block) {
                for(auto predicate : *(if_block)) {
                    if(predicate) {
                        std::cout<<predicate->resolve(this)<<std::endl;
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
                        std::cout<<expression->resolve(this)<<std::endl;
                    } else {
                        std::cout<<"nulltpr in then"<<std::endl;
                    }
                }
            } else {
                std::cout<<"empty (somehow)"<<std::endl;
            }
        }
    }

    void EIScriptExecutor::push_context(ScriptDeclaration* scriptDeclaration, ExpressionList* arguments) {
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

    void EIScriptExecutor::pop_context() {
        if(script_context->getParentContext() == nullptr) {
           cursedearth::ce_logging_error("Tried to pop parent context.");
        } else {
            EIScriptContext* previous = script_context;
            script_context = script_context->getParentContext();
            delete previous;
        }
    }


    ExpressionList EIScriptExecutor::resolveArguments(ExpressionList* arguments) {
        ExpressionList resolved_arguments(*arguments); //TODO is this even acceptable?
        std::transform(
            resolved_arguments.begin(), resolved_arguments.end(), resolved_arguments.begin(),
        [this](Expression* e) { if(e) return e->resolve(this); else return e; }
        );
        return resolved_arguments;
    }
}
