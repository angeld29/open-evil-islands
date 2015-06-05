#include "EIScriptContext.h"
#include "EIScriptClasses.h"

namespace EIScript
{

    template<class T>
    EIScriptContext<T>* EIScriptContext<T>::getParentContext()
    {
        return parent;
    }

    template<class T>
    EIScriptContext<T>* EIScriptContext<T>::extendedContext(VariableList* local_vars)
    {
        EIScriptContext<T>* extended = new EIScriptContext<T>(this);
        if(local_vars) {
            for(auto& var : *local_vars) {
                extended->addLocalVariable(var);
            }
        }
        return extended;
    }

    template<class T>
    void EIScriptContext<T>::clear_script()
    {
    }

    template<class T>
    void EIScriptContext<T>::addScript(ScriptDeclaration* script)
    {
        if(parent == nullptr) {
            scripts.insert(std::make_pair(*script->getName(), script));
        } else {
            throw EIScript::Exception::InvalidAction("cannot add script", "non-root script context");
        }
    }

    template<class T>
    void EIScriptContext<T>::addGlobalVariable(VariableDeclaration* variable)
    {
        globals.insert(std::make_pair(*variable->getName(), variable));
    }

    template<class T>
    void EIScriptContext<T>::addLocalVariable(VariableDeclaration* variable)
    {
        locals.insert(std::make_pair(*variable->getName(), variable));
    }

    template<class T>
    bool EIScriptContext<T>::functionDefined(Identifier* ident)
    {
        return ai_director->functionDefined(ident->name);
    }

    template<class T>
    bool EIScriptContext<T>::scriptDefined(Identifier* ident)
    {
        if(parent == nullptr) {
            return scripts.find(*(ident->name)) != scripts.end();
        } else {
            return parent->scriptDefined(ident);
        }
    }

    template<class T>
    bool EIScriptContext<T>::variableDefined(Identifier* ident)
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

    template<class T>
    Type EIScriptContext<T>::getFunctionType(Identifier* ident)
    {
        return ai_director->getFunctionType(ident->name);
    }

    template<class T>
    ScriptDeclaration* EIScriptContext<T>::getScript(Identifier* ident)
    {
        if(parent == nullptr) {
            return scripts[*ident->name];
        } else {
            return parent->getScript(ident);
        }
    }

    template<class T>
    VariableDeclaration* EIScriptContext<T>::getVariable(const Identifier* ident)
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

    template<class T>
    Expression* EIScriptContext<T>::callFunction(std::string* function_name, ExpressionList* arguments)
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

    template<class T>
    void EIScriptContext<T>::callScript(Identifier* function_name, ExpressionList* arguments)
    {
        ScriptDeclaration* script = getScript(function_name);
        if(!script && function_name == getWorldscript()->getId()) {
            script = getWorldscript();
        }

        script_executor->execute(script, arguments);
    }

    template<class T>
    void EIScriptContext<T>::dumpFunctions(std::ostream& str)
    {
        ai_director->dumpFunctions(str);
    }

    template<class T>
    void EIScriptContext<T>::dumpScripts(std::ostream& str)
    {
        if(parent == nullptr) {
            for(auto& pair : scripts) {
                str << "Script " << pair.first << " : " << pair.second << std::endl;
            }
        } else {
            return parent->dumpScripts(str);
        }

    }

    template<class T>
    void EIScriptContext<T>::dumpVariables(std::ostream& str)
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

    template class EIScriptContext<>;
}
