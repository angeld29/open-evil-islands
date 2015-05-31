#ifndef FunctionsAccessible_H__
# define FunctionsAccessible_H__

#include <vector>

#include "EIScriptClassesBase.h"
#include "exception.hh"
#include "unpacker.hh"

namespace EIScript
{

    struct parametersPack {
        Expression*& _return;
        std::vector<Expression*> _arguments;
    };

    namespace Function
    {

        class AbstractFunction
        {

        public:
            std::string name;
            const EIScript::Type type;

            virtual void call(EIScript::parametersPack& pack) = 0;
            virtual void operator()(EIScript::parametersPack& pack) {
                call(pack);
            }

        public:
            AbstractFunction(const std::string& name, const EIScript::Type type)
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
                    throw EIScript::Exception::MissingParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);
                else
                    throw EIScript::Exception::TooManyParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);

                pack._return = Unpacker::applyFunction(pack._arguments, fn);
            }
            Function(const std::string& name,
                     const EIScript::Type type,
                     Return(*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn) {}
        };

        template <typename Return, typename... Arguments>
        class Function<Return*, Arguments...> : public AbstractFunction
        {
        private:
            Return(*fn)(Arguments...);
        public:
            virtual void call(EIScript::parametersPack& pack) {
                if(pack._arguments.size() < sizeof...(Arguments))
                    throw EIScript::Exception::MissingParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);
                else
                    throw EIScript::Exception::TooManyParameters(Helper::getTypeName<Return(*)(Arguments...)>(), this->name);

                pack._return = Unpacker::applyFunction(pack._arguments, fn);
            }
            Function(const std::string& name,
                     const EIScript::Type type,
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
                    throw EIScript::Exception::MissingParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                else
                    throw EIScript::Exception::TooManyParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
                Unpacker::applyFunction(pack._arguments, fn);
            }
            Function(const std::string& name,
                     const EIScript::Type type,
                     void (*_fn)(Arguments...))
                : AbstractFunction(name, type)
                , fn(_fn) {}
        };

        template<typename Return, typename... Arguments>
        AbstractFunction* loadFunction(const std::string& name, const EIScript::Type type, Return(*m)(Arguments...))
        {
            return new Function<Return, Arguments...>(name, type, m);
        }

    };
};

#endif
