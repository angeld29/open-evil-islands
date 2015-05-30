#include "EIScriptContext.h"

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
            scripts.insert(std::make_pair(*script->id->name, script));
        } else {
            throw EIScript::Exception::InvalidAction("cannot add script", "non-root script context");
        }
    }

    void EIScriptContext::addGlobalVariable(VariableDeclaration* variable)
    {
        globals.insert(std::make_pair(*variable->id->name, variable));
    }

    void EIScriptContext::addLocalVariable(VariableDeclaration* variable)
    {
        locals.insert(std::make_pair(*variable->id->name, variable));
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

    ScriptDeclaration* EIScriptContext::getScript(Identifier* ident)
    {
        if(parent == nullptr) {
            return scripts[*ident->name];
        } else {
            return parent->getScript(ident);
        }
    }

    VariableDeclaration* EIScriptContext::getVariable(Identifier* ident)
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

    Expression* EIScriptContext::call(std::string* function_name, ExpressionList* arguments)
    {
        return ai_director->call(function_name, arguments);
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
