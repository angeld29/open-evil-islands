#ifndef Unpacker_H__
# define Unpacker_H__

#include <vector>
#include <string>

#include "EIScriptClassesBase.hpp"
#include "helper.hpp"
#include "scriptexception.hpp"

namespace EIScript
{
    namespace Converter
    {
        template <typename arg>
        struct convertExpression;

        template <typename arg>
        struct convertExpression { // int, double...etc
            static arg get(Expression* l) {
                ValuedExpression<arg>* _l =
                    dynamic_cast< ValuedExpression<arg>* >(l);
                if(!_l) {
                    throw EIScript::Exception::InvalidParameter(Helper::getTypeName<arg>(), ""); //l->getName());
                }
                return (_l->getValue());
            }
        };

        template <typename arg>
        struct convertExpression<arg&> { // int&, double&...etc
            static arg& get(Expression* l) {
                ValuedExpression<arg>* _l =
                    dynamic_cast< ValuedExpression<arg>* >(l);
                if(!_l)
                    throw EIScript::Exception::InvalidParameter(Helper::getTypeName<arg&>(), ""); //l->getName());
                return (_l->getValue());
            }
        };

    }

    namespace Unpacker
    {

        /* -------------------------- */
        template <int N, typename... T>
        struct parametersType;

        template <typename T0, typename... T>
        struct parametersType<0, T0, T...> {
            typedef T0 type;
        };

        template <int N, typename T0, typename... T>
        struct parametersType<N, T0, T...> {
            typedef typename parametersType<N-1, T...>::type type;
        };
        /* -------------------------- */

        template<unsigned int N>
        struct unfolder;

        template<unsigned int N>
        struct unfolder {
            template <typename ReturnType, typename... Arguments, typename ...final>
            static ReturnType applyFunc(std::vector<Expression*> parameters,
                                        ReturnType(*fn)(Arguments...),
                                        final&&... args) {
                return (unfolder<N - 1>::applyFunc
                        (parameters, fn,
                         Converter::convertExpression< typename parametersType<N - 1, Arguments...>::type >
                         ::get(parameters[N-1]),
                         args...));
            }

            template <typename ReturnType, typename Object, typename... Arguments, typename ...final>
            static ReturnType applyMethod(std::vector<Expression*> parameters,
                                          Object*	_this,
                                          ReturnType(Object::*fn)(Arguments...),
                                          final&&... args) {
                return (unfolder<N - 1>::applyMethod
                        (parameters, _this, fn,
                         Converter::convertExpression< typename parametersType<N - 1, Arguments...>::type >
                         ::get(parameters[N-1]),
                         args...));
            }
        };

        template<>
        struct unfolder<0> {
            template <typename ReturnType, typename ...Arguments, typename ...final>
            static ReturnType applyFunc(std::vector<Expression*>,
                                        ReturnType(*fn)(Arguments...),
                                        final&&... args) {
                return (fn(args...));
            }

            template <typename ReturnType, typename Object, typename ...Arguments, typename ...final>
            static ReturnType applyMethod(std::vector<Expression*>,
                                           Object*	_this,
                                           ReturnType(Object::*fn)(Arguments...),
                                           final&&... args) {
                return ((_this->*fn)(args...));
            }
        };


        template <typename ReturnType, typename ...Arguments>
        ReturnType applyFunction(std::vector<Expression*> args,
                                 ReturnType(*fn)(Arguments...))
        {
            return (unfolder<sizeof...(Arguments)>::applyFunc(args, fn));
        }

        template <typename Object, typename ReturnType, typename ...Arguments>
        ReturnType applyMethod(std::vector<Expression*> args,
                                Object* _this,
                                ReturnType(Object::*fn)(Arguments...))
        {
            return (unfolder<sizeof...(Arguments)>::applyMethod(args, _this, fn));
        }

    }
}

#endif
