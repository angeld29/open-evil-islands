#ifndef EI_SCRIPT_CLASSES_BASE
#define EI_SCRIPT_CLASSES_BASE

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace EIScript
{
    class Expression;
    class VariableDeclaration;
    class ScriptDeclaration;
    class EIScriptExecutor;

    typedef std::vector<Expression*> ExpressionList;
    typedef std::vector<VariableDeclaration*> VariableList;

    enum class Type { Float, String, Object, Group, Void };

    inline std::string typeToString(const Type& value)
    {
        static std::map<Type, std::string> strings;
        if(strings.size() == 0) {
#define INSERT_ELEMENT(p) strings[p] = #p
            INSERT_ELEMENT(Type::Float);
            INSERT_ELEMENT(Type::String);
            INSERT_ELEMENT(Type::Object);
            INSERT_ELEMENT(Type::Group);
            INSERT_ELEMENT(Type::Void);
#undef INSERT_ELEMENT
        }
        return strings[value];
    }

    inline std::ostream& operator<<(std::ostream& out, const Type& value)
    {
        return out << typeToString(value);
    }

    class Expression
    {
    public:
        Type getType() {
            return type;
        }

        virtual Expression* resolve(EIScriptExecutor*) {
            return this;
        }

        virtual void log(std::ostream& out) = 0;

        virtual ~Expression() {}
    protected:
        const Type type;
        Expression(Type type)
            : type(type) {
        }
    };

    template<typename T>
    class ValuedExpression : public Expression
    {

    public:
        T value;
        virtual T getValue() {
            return value;
        }

        virtual void log(std::ostream& out) {
            out<<value;
        }

        virtual ~ValuedExpression() {}
    protected:
        ValuedExpression(T value, Type type)
            : Expression(type)
            , value(value) {
        }
    };

    class FloatValue : public ValuedExpression<double>
    {
    public:
        FloatValue(double value)
            : ValuedExpression(value, Type::Float) {
        }
    };

    class StringValue : public ValuedExpression<std::string*>
    {
    public:
        StringValue(std::string* value)
            : ValuedExpression(value, Type::String) {
        }

        ~StringValue() {
            delete value;
        }
    };

    class ObjectValue : public ValuedExpression<int>
    {
    public:
        ObjectValue(int value)
            : ValuedExpression(value, Type::Object) {
        }
    };

    class GroupValue : public ValuedExpression<int>
    {
    public:
        GroupValue(int value)
            : ValuedExpression(value, Type::Group) {
        }
    };

    class Identifier
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
}

#endif // EI_SCRIPT_CLASSES_BASE
