#include "EIScriptContext.h"
#include "EIScriptClasses.h"

namespace EIScript
{
    EIScriptContext* EIScriptContext::getParentContext()
    {
        return parent;
    }

    EIScriptContext* EIScriptContext::extendedContext(VariableList* new_vars)
    {
        EIScriptContext* extended = new EIScriptContext(this);
        if(new_vars) {
            for(auto& var : *new_vars) {
                extended->addLocalVariable(var);
            }
        }
        return extended;
    }

    void EIScriptContext::clear_script()
    {
    }

    void EIScriptContext::addScript(ScriptDeclaration* script)
    {
        if(parent == nullptr) {
            scripts.insert(std::make_pair(*script->getName(), script));
        } else {
            throw EIScript::Exception::InvalidAction("cannot add script", "non-root script context");
        }
    }

    void EIScriptContext::addGlobalVariable(VariableDeclaration* variable)
    {
        globals.insert(std::make_pair(*variable->getName(), variable));
    }

    void EIScriptContext::addLocalVariable(VariableDeclaration* variable)
    {
        locals.insert(std::make_pair(*variable->getName(), variable));
    }


    bool EIScriptContext::functionDefined(Identifier* ident)
    {
        return ai_director->functionDefined(ident->name);
    }

    bool EIScriptContext::scriptDefined(Identifier* ident)
    {
        if(parent == nullptr) {
            return scripts.find(*(ident->name)) != scripts.end();
        } else {
            return parent->scriptDefined(ident);
        }
    }

    bool EIScriptContext::variableDefined(Identifier* ident)
    {
        if(locals.find(*(ident->name)) != locals.end()) {
            return true;
        } else {
            if(globals.find(*(ident->name)) != globals.end()) {
                return true;
            } else if(parent != nullptr) {
                return parent->variableDefined(ident);
            } else {
                return false;
            }
        }
    }

    Type EIScriptContext::getFunctionType(Identifier* ident)
    {
        return ai_director->getFunctionType(ident->name);
    }

    ScriptDeclaration* EIScriptContext::getScript(Identifier* ident)
    {
        if(parent == nullptr) {
            return scripts[*ident->name];
        } else {
            return parent->getScript(ident);
        }
    }

    VariableDeclaration* EIScriptContext::getVariable(const Identifier* ident)
    {
        VariableDeclaration* local = locals[*ident->name];
        if(!local) {
            VariableDeclaration* global = globals[*ident->name];
            if(!global) {
                if(parent != nullptr) {
                    return parent->getVariable(ident);
                } else {
                    return nullptr;
                }
            } else {
                return global;
            }
        } else {
            return local;
        }
    }

    Expression* EIScriptContext::callFunction(std::string* function_name, ExpressionList* arguments)
    {
        ExpressionList resolved_arguments(*arguments); //TODO is this even acceptable?
        std::transform(
            resolved_arguments.begin(), resolved_arguments.end(), resolved_arguments.begin(),
        [this](Expression* e) { if(e) return e->resolve(this); else return e; }
        );
        return ai_director->call(function_name, resolved_arguments);
    }

    /* DEBUG */

    std::ostream& operator << (std::ostream& os, std::nullptr_t)
    {
        return os << "nullptr";
    }

    /* END DEBUG */

    void EIScriptContext::callScript(Identifier* function_name, ExpressionList* arguments)
    {
        std::cout<<"Called script "<<*(function_name->name)<<std::endl;
        /* HEAVY WIP --- also ideally should be done through a ScriptExecutor of sime sorts */
        ScriptDeclaration* script = getScript(function_name);
        if(!script && function_name == getWorldscript()->getId()) {
            script = getWorldscript();
        }

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

    void EIScriptContext::dumpFunctions(std::ostream& str)
    {
        ai_director->dumpFunctions(str);
    }

    void EIScriptContext::dumpScripts(std::ostream& str)
    {
        if(parent == nullptr) {
            for(auto& pair : scripts) {
                str << "Script " << pair.first << " : " << pair.second << std::endl;
            }
        } else {
            return parent->dumpScripts(str);
        }

    }

    void EIScriptContext::dumpVariables(std::ostream& str)
    {
        for(auto& pair : globals) {
            str << "Global variable " << pair.first << " : " << pair.second << std::endl;
        }
        for(auto& pair : locals) {
            str << "Local variable " << pair.first << " : " << pair.second << std::endl;
        }
        if(parent != nullptr) {
            parent->dumpVariables(str);
        }
    }
}
