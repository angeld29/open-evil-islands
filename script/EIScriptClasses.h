
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "position.hh"

/* Scripts use the same FunctionDeclaration class for simplicity,
 * since the only distinction is the absence of a return type. */
namespace EIScript
{
    class Expression;
    class VariableDeclaration;
    class FunctionDeclaration;

    typedef std::vector<Expression*> ExpressionList;
    typedef std::vector<VariableDeclaration*> VariableList;

    enum class Type { Float, String, Object, Group, None };

    inline std::ostream& operator<<(std::ostream& out, const Type& value)
    {
        static std::map<Type, std::string> strings;
        if(strings.size() == 0) {
#define INSERT_ELEMENT(p) strings[p] = #p
            INSERT_ELEMENT(Type::Float);
            INSERT_ELEMENT(Type::String);
            INSERT_ELEMENT(Type::Object);
            INSERT_ELEMENT(Type::Group);
            INSERT_ELEMENT(Type::None);
#undef INSERT_ELEMENT
        }

        return out << strings[value];
    }

    class Expression
    {

    public:
        virtual Type getType() = 0;
        virtual ~Expression();
    };

    class FloatConstant : public Expression
    {
    public:
        double value;
        FloatConstant(double value)
            : value(value) {
        }

        virtual Type getType() {
            return Type::Float;
        }
    };

    class StringConstant : public Expression
    {
    public:
        std::string* value;
        StringConstant(std::string* value)
            : value(value) {
        }

        virtual Type getType() {
            return Type::String;
        }

        ~StringConstant() {
            delete value;
        }
    };

    class Identifier /* : public Expression */ // an identifier by itself is not an expression, I think
    {
    public:
        std::string* name;
        Identifier(std::string* name)
            : name(name) {
        }

        ~Identifier() {
            delete name;
        }
    };

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
