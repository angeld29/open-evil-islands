#include "AIDirector.h"

namespace cursedearth
{

    AIDirector::AIDirector()
    {
    }

    AIDirector::~AIDirector()
    {
    }

    EIScript::Expression* AIDirector::call(std::string* function_name, EIScript::ExpressionList* arguments)
    {
    }

    void AIDirector::addFunction(EIScript::FunctionDeclaration* function)
    {
        functions_impl->addFunction(function);
    }

    bool AIDirector::functionDefined(std::string* function_name)
    {
        functions_impl->functionDefined(function_name);
    }

    EIScript::FunctionDeclaration* AIDirector::getFunction(std::string* function_name)
    {
        functions_impl->getFunction(function_name);
    }

    void AIDirector::dumpFunctions(std::ostream& str)
    {
        functions_impl->dumpFunctions(str);
    }
    
}
