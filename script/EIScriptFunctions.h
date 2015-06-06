#ifndef EI_SCRIPT_FUNCTIONS
#define EI_SCRIPT_FUNCTIONS

#include <string>
#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"
#include "EIScriptFunctionsBase.h"
#include "function.hh"
#include "util.h"

namespace EIScript
{

    class EIScriptFunctions : public EIScriptFunctionsBase
    {
        typedef typename Function::AbstractFunction FunctionType;
    public:
        EIScriptFunctions(DirectorType* ai_director);

        ~EIScriptFunctions();

        virtual Expression* call(std::string* function_name, ExpressionList& arguments);

        virtual bool functionDefined(std::string* function_name);
        virtual Type getFunctionType(std::string* function_name);
        virtual void dumpFunctions(std::ostream& str);

    protected:
        boost::unordered_map<std::string, FunctionType*, CaseInsensitiveHash, CaseInsensitiveEqual> functions;

        void initFunctions();
    };

}

#endif // EI_SCRIPT_FUNCTIONS
