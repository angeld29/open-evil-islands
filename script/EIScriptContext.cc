#include "EIScriptContext.h"
#include "EIScriptClasses.h"

namespace EIScript
{

    EIScriptContext::EIScriptContext()
        : parent(nullptr) { }

    EIScriptContext::EIScriptContext(EIScriptContext* parent)
        : parent(parent) { }

    EIScriptContext::~EIScriptContext()
    {
        if(!parent) {
            delete worldscript;
        }
    }
    
    ScriptDeclaration* EIScriptContext::getWorldscript()
    {
        return worldscript;
    }

    void EIScriptContext::setWorldscript(ScriptDeclaration* worldscript)
    {
        this->worldscript = worldscript;
    }

    EIScriptContext* EIScriptContext::getParentContext()
    {
        return parent;
    }

    EIScriptContext* EIScriptContext::extendedContext(VariableList* local_vars)
    {
        EIScriptContext* extended = new EIScriptContext(this);
        if(local_vars) {
            for(auto& var : *local_vars) {
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
        return getScript(ident->name);
    }

    ScriptDeclaration* EIScriptContext::getScript(std::string* script_name)
    {
        if(parent == nullptr) {
            return scripts[*script_name];
        } else {
            return parent->getScript(script_name);
        }
    }

    VariableDeclaration* EIScriptContext::getVariable(const Identifier* ident)
    {
        return getVariable(ident->name);
    }
    
    VariableDeclaration* EIScriptContext::getVariable(std::string* var_name)
    {
        // TODO rewrite?
        VariableDeclaration* local = locals[*var_name];
        if(!local) {
            VariableDeclaration* global = globals[*var_name];
            if(!global) {
                if(parent != nullptr) {
                    return parent->getVariable(var_name);
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
