
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

enum class Type { Float, String, Object, Group };

inline
std::ostream& operator<<(std::ostream& out, const Type& value){
    static std::map<Type, std::string> strings;
    if (strings.size() == 0){
#define INSERT_ELEMENT(p) strings[p] = #p
        INSERT_ELEMENT(Type::Float);     
        INSERT_ELEMENT(Type::String);     
        INSERT_ELEMENT(Type::Object);             
        INSERT_ELEMENT(Type::Group);             
#undef INSERT_ELEMENT
    }   

    return out << strings[value];
}

class Node {
public:
    virtual ~Node() {}
};

class Expression : public Node {
};

class FloatExpr : public Expression {
public:
    double value;
    FloatExpr(double value) : value(value) { }
};

class StringExpr : public Expression {
public:
    std::string value;
    StringExpr(const std::string& value) : value(value) { }
};

class Identifier : public Expression {
public:
    std::string name;
    Identifier(const std::string& name) : name(name) { }
};

class FunctionCall : public Expression {
public:
    const Identifier& id;
    ExpressionList arguments;
    FunctionCall(const Identifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    FunctionCall(const Identifier& id) : id(id) { }
};


class Assignment : public Expression {
public:
    Identifier& lhs;
    Expression& rhs;
    Assignment(Identifier& lhs, Expression& rhs) : 
        lhs(lhs), rhs(rhs) { }
};

class VariableDeclaration : public Node {
public:
    const Identifier& type;
    Identifier& id;
    Expression *assignmentExpr;
    VariableDeclaration(const Identifier& type, Identifier& id) :
        type(type), id(id) { }
    VariableDeclaration(const Identifier& type, Identifier& id, Expression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { }
};

class FunctionDeclaration : public Node {
public:
    const Type type;
    const Type returnType;
    const Identifier& id;
    VariableList arguments;
    FunctionDeclaration(const Type type, const Type returnType, Identifier& id, 
            const VariableList& arguments) :
        type(type), returnType(returnType), id(id), arguments(arguments) { }
};