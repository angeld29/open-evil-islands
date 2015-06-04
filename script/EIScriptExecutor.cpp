#include "EIScriptExecutor.h"

namespace EIScript
{

    void EIScriptExecutor::execute(ScriptContext* script_context, ScriptDeclaration* script, ExpressionList* arguments)
    {
        Identifier* function_name = script->getId();
        for(auto block : *(script->getScriptBody())) {
            auto if_block = std::get<0>(*block);
            auto then_block = std::get<1>(*block);

            std::cout<<"IF in "<<*(function_name->name)<<std::endl;
            if(if_block) {
                for(auto predicate : *(if_block)) {
                    if(predicate) {
                        std::cout<<predicate->resolve(script_context)<<std::endl;
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
                        std::cout<<expression->resolve(script_context)<<std::endl;
                    } else {
                        std::cout<<"nulltpr in then"<<std::endl;
                    }
                }
            } else {
                std::cout<<"empty (somehow)"<<std::endl;
            }
        }
    }


}
