
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "EIScriptClassesBase.h"
#include "position.hh"

/* Scripts use the same FunctionDeclaration class for simplicity,
 * since the only distinction is the absence of a return type. */
namespace EIScript
{
    class Assignment : public Expression
    {
    public:
        Identifier* lhs;
        Expression* rhs;
        Assignment(Identifier* lhs, Expression* rhs)
            : lhs(lhs)
            , rhs(rhs) {
        }

        virtual Type getType() {
            return Type::None;
        }

        ~Assignment() {
            delete rhs;
        }
    };

    class VariableDeclaration
    {
    public:
        const Type type;
        Identifier* id;
        Expression* assignedExpr;

        position* defined_at;

        VariableDeclaration(const Type type, Identifier* id)
            : type(type)
            , id(id) {
        }
        VariableDeclaration(const Type type, Identifier* id, Expression* assignedExpr)
            : type(type)
            , id(id)
            , assignedExpr(assignedExpr) {
        }

        ~VariableDeclaration() {
            delete id; //?
            delete assignedExpr;
        }
    };

    class FunctionDeclaration
    {
    public:
        const Type returnType;
        const Identifier* id;
        VariableList* arguments;

        FunctionDeclaration(const Type returnType, const Identifier* id, VariableList* arguments)
            : returnType(returnType)
            , id(id)
            , arguments(arguments) {
        }

        ~FunctionDeclaration() {
            delete id; //?
            delete arguments;
        }
    };

    class FunctionCall : public Expression
    {
    public:
        const FunctionDeclaration* func;
        ExpressionList* arguments;
        FunctionCall(const FunctionDeclaration* func, ExpressionList* arguments)
            : func(func)
            , arguments(arguments) {
        }
        FunctionCall(const FunctionDeclaration* func)
            : func(func) {
        }

        virtual Type getType() {
            return func->returnType;
        }

        ~FunctionCall() {
            delete arguments;
        }
    };
}
