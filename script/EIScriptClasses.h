
#pragma once

#include <tuple>
#include <utility>

#include "EIScriptClassesBase.h"
#include "position.hh"
#include "exception.hh"

namespace EIScript
{
    typedef tuple<ExpressionList*, ExpressionList*> ScriptBlock;
    typedef std::vector<ScriptBlock*> ScriptBody;

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

        virtual Expression* resolve(EIScriptContext* context);

        ~Assignment() {
            //delete rhs;
        }
    };

    class VariableDeclaration
    {
    public:
        const Type type;

        VariableDeclaration(Type type, Identifier* id)
            : type(type)
            , id(id) {
        }

        Expression* getValue() {
            return value;
        }

        void setValue(Expression* e) {
            if(e->getType() != type) {
                throw Exception::InvalidAction(
                    "wrong variable type: " + typeToString(e->getType()),
                    "variable " + *(id->name) + " of type " + typeToString(type)
                );
            } else {
                value = e;
            }
        }

        ~VariableDeclaration() {
            delete id; //?
        }
    protected:
        Identifier* id;
        Expression* value;
        position* defined_at;
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

    class ScriptDeclaration : public BaseSubRoutine
    {
    public:
        ScriptDeclaration(const Identifier* id, VariableList* arguments)
            : BaseSubRoutine(id, arguments) {
        }

        void setScriptBody(ScriptBody* body) {
            this->body = body;
        }

//        void addScriptBlock(ScriptBlock* block){
//            body.push_back(block);
//        }

    protected:
        ScriptBody* body;
    };

    class FunctionCall : public Expression
    {
    public:
        const std::string* functionName;
        ExpressionList* arguments;

        FunctionCall(const std::string* functionName, ExpressionList* arguments, const Type type)
            : Expression(type)
            , func(func)
            , arguments(arguments) {
        }

        FunctionCall(const FunctionDeclaration* func)
            : Expression(const Type type)
            , func(func) {
        }

        virtual Expression* resolve(EIScriptContext* context);

        ~FunctionCall() {
            delete arguments;
        }
    protected:
    };
}
