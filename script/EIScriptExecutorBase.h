#ifndef EI_SCRIPT_EXECUTOR_BASE
#define EI_SCRIPT_EXECUTOR_BASE

#include "EIScriptClassesBase.h"
#include "EIScriptClasses.h"

namespace EIScript
{

    // TODO keep the context here
    template<typename T>
    class EIScriptExecutorBase
    {
    protected:
        typedef EIScriptContext<T> ScriptContext;
    public:
        EIScriptExecutorBase() {}
        virtual ~EIScriptExecutorBase() {}
        
        virtual void advance(ScriptContext* script_context) = 0;
        virtual void execute(ScriptDeclaration* script, ExpressionList* arguments) = 0;
    };

}

#endif // EI_SCRIPT_EXECUTOR_BASE
