#ifndef FunctionsAccessible_H__
#define FunctionsAccessible_H__

#include <vector>
#include <type_traits>

#include "EIScriptClassesBase.hpp"
#include "ScriptAccessible.hpp"
#include "scriptexception.hpp"
#include "unpacker.hpp"

namespace EIScript
{

    struct parametersPack {
        Expression* _return;
        std::vector<Expression*>& _arguments;

        parametersPack(std::vector<Expression*>& _arguments)
            : _arguments(_arguments) {}
    };

    namespace Function
    {

        class AbstractFunction
        {

        public:
            std::string name;
            const Type type;

            virtual void call(EIScript::parametersPack& pack) = 0;
            virtual void operator()(EIScript::parametersPack& pack) {
                call(pack);
            }

        public:
            AbstractFunction(const std::string& name, const Type type)
                : name(name)
                , type(type) {}
            virtual ~AbstractFunction() {}

        };

        template <typename Return, typename... Arguments>
        class Function : public AbstractFunction
        {
        private:
            Return(*fn)(Arguments...);
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw Exception::MissingParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);
                else if(pack._arguments.size() > sizeof...(Arguments))
                    throw Exception::TooManyParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);

                pack._return = Helper::wrapper<Return>::wrap(Unpacker::applyFunction(pack._arguments, fn), type);
            }
            Function(const std::string& name,
                     const Type type,
                     Return(*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn) {}
        };

        template <typename Return, typename... Arguments>
        class Function<Return*, Arguments...> : public AbstractFunction
        {
        private:
            Return(*fn)(Arguments...); //TODO shouldn't it be Return* here?
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw Exception::MissingParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);
                else if(pack._arguments.size() > sizeof...(Arguments))
                    throw Exception::TooManyParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);

                pack._return = Helper::wrapper<Return*>::wrap(Unpacker::applyFunction(pack._arguments, fn), type);
            }
            Function(const std::string& name,
                     const Type type,
                     Return* (*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn) {}
        };

        template <typename... Arguments>
        class Function<void, Arguments...> : public AbstractFunction
        {
        private:
            void (*fn)(Arguments...);
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw Exception::MissingParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                else if(pack._arguments.size() > sizeof...(Arguments))
                    throw Exception::TooManyParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                Unpacker::applyFunction(pack._arguments, fn);
                pack._return = nullptr;
            }
            Function(const std::string& name,
                     const Type type,
                     void (*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn) {}
        };

        template<typename Return, typename... Arguments>
        AbstractFunction* loadFunction(const std::string& name, const Type type, Return(*m)(Arguments...))
        {
            return new Function<Return, Arguments...>(name, type, m);
        }

        //-----------------------------------------------------------

        template <typename Object, typename Return, typename... Arguments>
        class Method : public AbstractFunction
        {
        private:
            Return(Object::*fn)(Arguments...);
            Object* _this; //TODO actually const? //TODO via parameter pack? (to allow for object selection)
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw Exception::MissingParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                else if(pack._arguments.size() > sizeof...(Arguments))
                    throw Exception::TooManyParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                pack._return = Helper::wrapper<Return>::wrap(Unpacker::applyMethod(pack._arguments, _this, fn), type);
            }
            Method(const std::string& name,
                   const Type type,
                   Object* _this,
                   Return(Object::*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn)
                , _this(_this) {}
        };

        template <typename Object, typename Return, typename... Arguments>
        class Method<Object, Return*, Arguments...> : public AbstractFunction
        {
        private:
            Return*(Object::*fn)(Arguments...);
            Object* _this;
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw Exception::MissingParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                else if(pack._arguments.size() > sizeof...(Arguments))
                    throw Exception::TooManyParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                pack._return = Helper::wrapper<Return*>::wrap(Unpacker::applyMethod(pack._arguments, _this, fn), type);
            }
            Method(const std::string& name,
                   const Type type,
                   Object* _this,
                   Return* (Object::*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn)
                , _this(_this) {}
        };

        template <typename Object, typename... Arguments>
        class Method<Object, void, Arguments...> : public AbstractFunction
        {
        private:
            void (Object::*fn)(Arguments...);
            Object* _this;
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw Exception::MissingParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                else if(pack._arguments.size() > sizeof...(Arguments))
                    throw Exception::TooManyParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                Unpacker::applyMethod(pack._arguments, _this, fn);
                pack._return = nullptr;
            }
            Method(const std::string& name,
                   const Type type,
                   Object* _this,
                   void (Object::*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn)
                , _this(_this) {}
        };

        template<bool> struct MakeMethod {};

        template<>
        struct MakeMethod<true> {
            template<typename Return, typename Object, typename... Arguments>
            static AbstractFunction* get(const std::string& name, const Type type, Object* _this, Return(Object::*m)(Arguments...)) {
                return new Method<Object, Return, Arguments...>(name, type, _this, m);
            }
        };

        template<>
        struct MakeMethod<false> {
            template<typename Return, typename Object, typename... Arguments>
            static AbstractFunction* get(const std::string&, const Type, Object*, Return(Object::*)(Arguments...)) {
                //TODO better motivation for this requirement
                static_assert(std::is_base_of<ScriptAccessible, Object>::value == true, "[EIScript] Object must inherit from ScriptAccessible to have its methods exposed to the script");
                return nullptr;
            }
        };

        template<typename Return, typename Object, typename... Arguments>
        AbstractFunction* loadMethod(const std::string& name, const Type type, Object* _this, Return(Object::*m)(Arguments...))
        {
            return MakeMethod<std::is_base_of<ScriptAccessible, Object>::value>::get(name, type, _this, m);
        }


    }
}

#endif
