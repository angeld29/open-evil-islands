#ifndef EI_SCRIPT_EXECUTOR
#define EI_SCRIPT_EXECUTOR

#include <queue>
#include <tuple>
#include "EIScriptExecutorBase.h"
#include "log.h"

namespace EIScript
{

    class EIScriptExecutor : public EIScriptExecutorBase<EIScriptExecutor>
    {
    public:
        EIScriptExecutor() {}
        virtual ~EIScriptExecutor() {}

        virtual void advance(ScriptContext* script_context);
        virtual void execute(ScriptDeclaration* script, ExpressionList* arguments);
    protected:
        typedef std::tuple<ScriptDeclaration*, ExpressionList*> QueueEntry;
        std::queue<QueueEntry> script_queue;
        
        void doExecuteScript(ScriptContext* script_context, ScriptDeclaration* script, ExpressionList* arguments);
    };

}

#endif // EI_SCRIPT_EXECUTOR