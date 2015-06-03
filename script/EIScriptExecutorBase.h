#ifndef EI_SCRIPT_EXECUTOR_BASE
#define EI_SCRIPT_EXECUTOR_BASE

#include "EIScriptClassesBase.h"
#include "EIScriptClasses.h"

namespace EIScript
{

    class EIScriptExecutorBase
    {
    public:
        EIScriptExecutorBase() {}
        virtual ~EIScriptExecutorBase() {}
        
        virtual void execute(ScriptDeclaration* script, ExpressionList* arguments) = 0;
    };

}

#endif // EI_SCRIPT_EXECUTOR_BASE
