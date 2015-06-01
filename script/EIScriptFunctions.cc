#include "EIScriptFunctions.h"
#include "exception.hh"

namespace EIScript
{

    namespace EIStdlib
    {
        double Add(double x, double y)
        {
            return x + y;
        }

        double Sub(double x, double y)
        {
            return x - y;
        }

        double Mul(double x, double y)
        {
            return x * y;
        }

        double Div(double x, double y)
        {
            return x / y;
        }

        double IsEqual(double x, double y)
        {
            return x == y ? 1 : 0; //TODO epsilon-based comparison needed?
        }

        double IsGreater(double x, double y)
        {
            return x > y ? 1 : 0;
        }

        double IsLess(double x, double y)
        {
            return x < y ? 1 : 0;
        }
    }

    EIScriptFunctions::EIScriptFunctions(DirectorType* ai_director)
        : cursedearth::EIScriptFunctionsBase<EIScriptFunctions>(ai_director)
    {
        initFunctions();
    }

    EIScriptFunctions::~EIScriptFunctions()
    {
    }

    EIScript::Expression* EIScriptFunctions::call(std::string* function_name, ExpressionList& arguments)
    {
        FunctionType* f = functions[*function_name];
        if(!f) {
            std::cerr<<"[EIScriptFunctions] Called unknown function: "<<*function_name<<std::endl;
            return nullptr;
        } else {
            parametersPack pack(arguments); //TODO does this make a copy?
            f->call(pack);
            return pack._return;
        }
    }

    bool EIScriptFunctions::functionDefined(std::string* function_name)
    {
        return functions.find(*function_name) != functions.end();
    }

    EIScript::Type EIScriptFunctions::getFunctionType(std::string* function_name)
    {
        FunctionType* f = functions[*function_name];
        if(!f) {
            throw EIScript::Exception::InvalidAction("Function " + *function_name + " is not defined", "EIScriptFunctions");
        } else {
            return f->type;
        }
    }

    void EIScriptFunctions::dumpFunctions(std::ostream& str)
    {
        for(auto& pair : functions) {
            str << "[EIScriptFunctions] Function " << pair.first << " : " << pair.second << std::endl;
        }
    }

    void EIScriptFunctions::initFunctions()
    {
        using namespace EIStdlib;
#define REGISTER_BY_NAME(name, type) functions[#name] = Function::loadFunction(#name, type, name)
        REGISTER_BY_NAME(Add, Type::Float);
        REGISTER_BY_NAME(Sub, Type::Float);
        REGISTER_BY_NAME(Mul, Type::Float);
        REGISTER_BY_NAME(Div, Type::Float);
        REGISTER_BY_NAME(IsEqual, Type::Float);
        REGISTER_BY_NAME(IsGreater, Type::Float);
        REGISTER_BY_NAME(IsLess, Type::Float);
#undef REGISTER_BY_NAME
    }

}
