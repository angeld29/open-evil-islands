
#pragma once

#include <string>
#include <boost/unordered_map.hpp>
#include "AIDirector.h"
#include "EIScriptClassesBase.h"
#include "EIScriptExecutorBase.h"
#include "EIScriptExecutor.h"
#include "exception.hh"
#include "util.h"
#include "log.h"

namespace EIScript
{
    class VariableDeclaration;
    class ScriptDeclaration;

    // TODO figure out AIDirector template arguments
    template<class ScriptExecutor>
    class EIScriptContext
    {
    public:
        EIScriptContext(cursedearth::AIDirector<>* ai_director)
            : parent(nullptr)
            , ai_director(ai_director)
            , script_executor(new ScriptExecutor())
            , verbose_execution(true) {
        }

        EIScriptContext(EIScriptContext* parent)
            : parent(parent)
            , ai_director(parent->ai_director)
            , script_executor(parent->script_executor)
            , verbose_execution(parent->verbose_execution) {
        }

        EIScriptContext* getParentContext();
        EIScriptContext* extendedContext(VariableList* new_vars);

        void clear_script();

        void addScript(ScriptDeclaration* script);
        void addGlobalVariable(VariableDeclaration* variable);
        void addLocalVariable(VariableDeclaration* variable);

        bool functionDefined(Identifier* ident);
        bool scriptDefined(Identifier* ident);
        bool variableDefined(Identifier* ident);

        Type getFunctionType(Identifier* ident);
        ScriptDeclaration* getScript(Identifier* ident);
        VariableDeclaration* getVariable(const Identifier* ident);

        Expression* callFunction(std::string* function_name, ExpressionList* arguments);
        void callScript(Identifier* function_name, ExpressionList* arguments);

        ScriptDeclaration* getWorldscript() {
            return worldscript;
        }

        void setWorldscript(ScriptDeclaration* worldscript) {
            this->worldscript = worldscript;
        }

        void dumpFunctions(std::ostream& str);
        void dumpScripts(std::ostream& str);
        void dumpVariables(std::ostream& str);

        void setVerboseExecution(bool verbose) {
            verbose_execution = verbose;
        }

        inline bool getVerboseExecution() {
            return verbose_execution;
        }

        //TODO temporary
        inline void advance() {
            script_executor->advance(this);
        }

        ~EIScriptContext() {
            if(!parent) {
                delete script_executor;
                delete worldscript;
            }
        }

    protected:
        EIScriptContext<ScriptExecutor>* parent;
        cursedearth::AIDirector<>* ai_director;
        // TODO extract
        EIScriptExecutorBase<ScriptExecutor>* script_executor;
        boost::unordered_map<std::string, ScriptDeclaration*> scripts;
        boost::unordered_map<std::string, VariableDeclaration*> globals;
        boost::unordered_map<std::string, VariableDeclaration*> locals;
        ScriptDeclaration* worldscript;

        bool verbose_execution;
    };
}
