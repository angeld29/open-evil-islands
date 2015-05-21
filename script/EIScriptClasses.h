
#pragma once

#include "EIScriptClassesBase.h"
#include "position.hh"

namespace EIScript
{
    class Assignment : public Expression
    {
    public:
        Identifier* lhs;
        Expression* rhs;
        Assignment(Identifier* lhs, Expression* rhs)
            : Expression(Type::None)
            , lhs(lhs)
            , rhs(rhs) {
        }

        //void apply(EIScriptContext* context) {

        //}

        ~Assignment() {
            delete rhs;
        }
    };

    class VariableDeclaration
    {
    public:
        const Type type;
        Identifier* id;
        position* defined_at;

        VariableDeclaration(Type type, Identifier* id)
            : type(type)
            , id(id) {
        }

        ~VariableDeclaration() {
            delete id; //?
        }
    };

    class BaseSubRoutine
    {
    public:
        const Identifier* id;
        VariableList* arguments;

        BaseSubRoutine(const Identifier* id, VariableList* arguments)
            : id(id)
            , arguments(arguments) {
        }

        virtual ~BaseSubRoutine() {
            delete id; //?
            delete arguments;
        }
    };

    class FunctionDeclaration : public BaseSubRoutine
    {
    public:
        const Type returnType;

        FunctionDeclaration(Type returnType, const Identifier* id, VariableList* arguments)
            : BaseSubRoutine(id, arguments)
            , returnType(returnType) {
        }
    };
    
    class ScriptDeclaration : public BaseSubRoutine
    {
    public:
        ScriptDeclaration(const Identifier* id, VariableList* arguments)
            : BaseSubRoutine(id, arguments) {
        }
    };

    template<typename T>
    class FunctionCall : public ValuedExpression<T>
    {
    public:
        const FunctionDeclaration* func;
        ExpressionList* arguments;

        FunctionCall(const FunctionDeclaration* func, ExpressionList* arguments)
            : ValuedExpression<T>(initIrrelevantValue(), func->returnType)
            , func(func)
            , arguments(arguments) {
        }

        FunctionCall(const FunctionDeclaration* func)
            : ValuedExpression<T>(initIrrelevantValue(), func->returnType)
            , func(func) {
        }

        //virtual T getValue() {
        //    return value;
        //}

        ~FunctionCall() {
            delete arguments;
        }
    protected:
        T initIrrelevantValue() {
            return *(new T());
        }
    };
}
