#ifndef EI_SCRIPT_CLASSES_BASE
#define EI_SCRIPT_CLASSES_BASE

#include "EIScriptContext.h"

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
        virtual void apply(EIScriptContext* context) = 0;
    };

    class FloatValue : public Expression
    {
    public:
        double value;
        FloatValue(double value)
            : value(value) {
        }

        virtual Type getType() {
            return Type::Float;
        }
        
        virtual void apply(EIScriptContext* context) { }
    };

    class StringValue : public Expression
    {
    public:
        std::string* value;
        StringValue(std::string* value)
            : value(value) {
        }

        virtual Type getType() {
            return Type::String;
        }
        
        virtual void apply(EIScriptContext* context) { }

        ~StringValue() {
            delete value;
        }
    };

    class ObjectValue : public Expression
    {
    public:
        int object_index;
        ObjectValue(int object_index)
            : object_index(object_index) {
        }

        virtual Type getType() {
            return Type::Object;
        }
        
        virtual void apply(EIScriptContext* context) { }
    };

    class GroupValue : public Expression
    {
    public:
        int group_index;
        GroupValue(int group_index)
            : group_index(group_index) {
        }

        virtual Type getType() {
            return Type::Group;
        }
        
        virtual void apply(EIScriptContext* context) { }
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
}

#endif // EI_SCRIPT_CLASSES_BASE
