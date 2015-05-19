
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

/* Scripts are called functions for simplicity, 
 * since the only distinction is the absence of a return type. */

class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;

enum class Type { Float, String, Object, Group. None };

inline
std::ostream& operator<<(std::ostream& out, const Type& value){
    static std::map<Type, std::string> strings;
    if (strings.size() == 0){
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

class Expression {
};

class FloatExpr : public Expression {
public:
    double value;
    FloatExpr(double value) : value(value) { }
};

class StringExpr : public Expression {
public:
    std::string* value;
    StringExpr(std::string* value) : value(value) { }
};

class Identifier : public Expression {
public:
    std::string* name;
    Identifier(std::string* name) : name(name) { }
};

class FunctionCall : public Expression {
public:
    const FunctionDeclaration* id;
    ExpressionList* arguments;
    FunctionCall(const FunctionDeclaration* id, ExpressionList* arguments) :
        id(id), arguments(arguments) { }
    FunctionCall(const FunctionDeclaration* id) : id(id) { }
};


class Assignment : public Expression {
public:
    Identifier* lhs;
    Expression* rhs;
    Assignment(Identifier* lhs, Expression* rhs) : 
        lhs(lhs), rhs(rhs) { }
};

class VariableDeclaration {
public:
    const Type type;
    Identifier* id;
    Expression* assignedExpr;
    VariableDeclaration(const Type type, Identifier* id) :
        type(type), id(id) { }
    VariableDeclaration(const Type type, Identifier* id, Expression* assignedExpr) :
        type(type), id(id), assignedExpr(assignedExpr) { }
};

class FunctionDeclaration {
public:
    const Type type;
    const Type returnType;
    const Identifier* id;
    VariableList* arguments;
    FunctionDeclaration(const Type type, const Type returnType, const Identifier* id, 
            VariableList* arguments) :
        type(type), returnType(returnType), id(id), arguments(arguments) { }
};