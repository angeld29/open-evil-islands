
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
        out<<"IF"<<std::endl;
        if(if_block) {
            out<<*if_block;
        }
        auto then_block = std::get<1>(value);
        out<<"THEN"<<std::endl;
        if(then_block) {
            out<<*then_block;
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, ScriptBody& value)
    {
        for(auto ptr : value) {
            if(ptr) {
                out<<*ptr;
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

        virtual Expression* resolve(EIScriptExecutor* executor);

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
            if(!e) {
                value = e;
                std::cout<<"Assigned nullptr to "<<*(id->name)<<std::endl;
                return;
            }
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

        virtual Expression* resolve(EIScriptExecutor* executor);

        virtual void log(std::ostream& out) ;

    private:
        const Identifier* id;
    };

    class ScriptDeclaration
    {
    public:

        ScriptDeclaration(Identifier* id, VariableList* arguments)
            : id(id)
            , arguments(arguments) {
        }

        //TODO decide which one to keep
        std::string* getName() {
            return id->name;
        }

        //TODO decide which one to keep
        Identifier* getId() {
            return id;
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
        //TODO figure out const correctness
        Identifier* id; //const Identifier...
        VariableList* arguments;
        ScriptBody* body;
    };

    class FunctionCall : public Expression
    {
    public:

        FunctionCall(Identifier* functionName, ExpressionList* arguments, const Type type)
            : Expression(type)
            , functionName(functionName)
            , arguments(arguments) {
        }

        FunctionCall(Identifier* functionName, const Type type)
            : Expression(type)
            , functionName(functionName) {
        }

        virtual Expression* resolve(EIScriptExecutor* executor);

        virtual void log(std::ostream& out) ;

        ~FunctionCall() {
            for(auto ptr : *arguments) {
                delete ptr;
            }
            delete arguments;
        }

    protected:
        Identifier* functionName;
        ExpressionList* arguments;
    };

    class ScriptCall : public FunctionCall
    {
    public:

        ScriptCall(Identifier* functionName, ExpressionList* arguments)
            : FunctionCall(functionName, arguments, Type::None) {
        }

        ScriptCall(Identifier* functionName)
            : FunctionCall(functionName, Type::None) {
        }

        virtual Expression* resolve(EIScriptExecutor* executor);

        virtual void log(std::ostream& out);

    };

}
