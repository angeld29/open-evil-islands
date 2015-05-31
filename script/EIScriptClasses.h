
#pragma once

#include <tuple>
#include <utility>

#include "EIScriptClassesBase.h"
#include "position.hh"
#include "exception.hh"

namespace EIScript
{
    typedef std::tuple<ExpressionList*, ExpressionList*> ScriptBlock;
    typedef std::vector<ScriptBlock*> ScriptBody;

    /* -------------------------------------------------- */
    inline std::ostream& operator<<(std::ostream& out, Expression& value)
    {
        value.log(out);
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, ExpressionList& value)
    {
        for(auto ptr : value) {
            if(ptr) {
                out<<*ptr<<std::endl;
            }
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, ScriptBlock& value)
    {
        auto if_block = std::get<0>(value);
        if(if_block) {
            out<<"IF"<<std::endl;
            out<<*if_block;
        }
        auto then_block = std::get<1>(value);
        if(then_block) {
            out<<"THEN"<<std::endl;
            out<<*then_block;
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, ScriptBody& value)
    {
        for(auto ptr : value) {
            if(ptr) {
                out<<*(dynamic_cast<ScriptBlock*>(ptr));
            }
        }
        return out;
    }
    /* -------------------------------------------------- */

    class Assignment : public Expression
    {
    public:
        Assignment(Identifier* lhs, Expression* rhs)
            : Expression(Type::None)
            , lhs(lhs)
            , rhs(rhs) {
        }

        virtual Expression* resolve(EIScriptContext* context);

        virtual void log(std::ostream& out);

        ~Assignment() {
            //delete rhs;
        }
    private:
        Identifier* lhs;
        Expression* rhs;
    };

    class VariableDeclaration
    {
    public:
        const Type type;

        VariableDeclaration(Type type, Identifier* id)
            : type(type)
            , id(id) {
        }

        std::string* getName() {
            return id->name;
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
    private:
        Identifier* id;
        Expression* value;
        position* defined_at;
    };

    class VariableAccess : public Expression
    {
    public:

        VariableAccess(const Identifier* id, const Type type)
            : Expression(type)
            , id(id) {
        }

        virtual Expression* resolve(EIScriptContext* context);

        virtual void log(std::ostream& out) ;

    private:
        const Identifier* id;
    };

    class ScriptDeclaration
    {
    public:

        ScriptDeclaration(const Identifier* id, VariableList* arguments)
            : id(id)
            , arguments(arguments) {
        }

        std::string* getName() {
            return id->name;
        }

        VariableList* getArguments() {
            return arguments;
        }

        ScriptBody* getScriptBody() {
            return body;
        }

        void setScriptBody(ScriptBody* body) {
            this->body = body;
        }

//        void addScriptBlock(ScriptBlock* block){
//            body.push_back(block);
//        }

        virtual ~ScriptDeclaration() {
            delete id; //?
            for(auto ptr : *arguments) {
                delete ptr;
            }
            delete arguments;
        }

    private:
        const Identifier* id;
        VariableList* arguments;
        ScriptBody* body;
    };

    class FunctionCall : public Expression
    {
    public:

        FunctionCall(const Identifier* functionName, ExpressionList* arguments, const Type type)
            : Expression(type)
            , functionName(functionName)
            , arguments(arguments) {
        }

        FunctionCall(const Identifier* functionName, const Type type)
            : Expression(type)
            , functionName(functionName) {
        }

        virtual Expression* resolve(EIScriptContext* context);

        virtual void log(std::ostream& out) ;

        ~FunctionCall() {
            for(auto ptr : *arguments) {
                delete ptr;
            }
            delete arguments;
        }

    protected:
        const Identifier* functionName;
        ExpressionList* arguments;
    };

    class ScriptCall : public FunctionCall
    {
    public:

        ScriptCall(const Identifier* functionName, ExpressionList* arguments)
            : FunctionCall(functionName, arguments, Type::None) {
        }

        ScriptCall(const Identifier* functionName)
            : FunctionCall(functionName, Type::None) {
        }

        virtual Expression* resolve(EIScriptContext* context);

        virtual void log(std::ostream& out);

    };

}
