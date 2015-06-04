#ifndef EI_SCRIPT_EXECUTOR_BASE
#define EI_SCRIPT_EXECUTOR_BASE

#include "EIScriptClassesBase.h"
#include "EIScriptClasses.h"

namespace EIScript
{

    template<typename T>
    class EIScriptExecutorBase
    {
    protected:
        typedef EIScriptContext<T> ScriptContext;
    public:
        EIScriptExecutorBase() {}
        virtual ~EIScriptExecutorBase() {}
        
        virtual void execute(ScriptContext* script_context, ScriptDeclaration* script, ExpressionList* arguments) = 0;
    };

}

#endif // EI_SCRIPT_EXECUTOR_BASE
