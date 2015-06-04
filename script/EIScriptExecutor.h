#ifndef EI_SCRIPT_EXECUTOR
#define EI_SCRIPT_EXECUTOR

#include "EIScriptExecutorBase.h"
#include "log.h"

namespace EIScript
{

    class EIScriptExecutor : public EIScriptExecutorBase<EIScriptExecutor>
    {
    public:
        EIScriptExecutor() {}
        virtual ~EIScriptExecutor() {}

        virtual void execute(ScriptContext* script_context, ScriptDeclaration* script, ExpressionList* arguments);
    };

}

#endif // EI_SCRIPT_EXECUTOR
