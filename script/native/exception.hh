#ifndef Error_h__
# define Error_h__

#include <stdexcept>
#include <string>

namespace EIScript
{
    namespace Exception
    {
        class InvalidAction : public std::exception
        {
        private:
            std::string action;
            std::string name;

        public:
            InvalidAction(const std::string& _c, const std::string& _n)
                : action(_c), name(_n) {}
            virtual ~InvalidAction() throw() {}

        public:
            virtual const char* what() throw() {
                return (std::string("Invalid Action : "
                                    + action
                                    + " For Object {"+ name +"} "
                                    + causedBy()).c_str());
            }

        protected:
            virtual const std::string causedBy() { return ""; }
        };

        class InvalidParameter : public InvalidAction
        {
        public:
            InvalidParameter(const std::string& type, const std::string& _n)
                : InvalidAction("cast object to " + type , _n) {}
        };

        class MissingParameters : public InvalidAction
        {
        private:
            std::string prototype;

        public:
            MissingParameters(const std::string& proto, const std::string& name)
                : InvalidAction("call()", name), prototype(proto) {}
            virtual ~MissingParameters() throw() {}

        protected:
            virtual const std::string causedBy() { return "Not enough arguments"; }
        };


        class TooManyParameters : public InvalidAction
        {
        private:
            std::string prototype;

        public:
            TooManyParameters(const std::string& proto, const std::string& name)
                : InvalidAction("call()", name), prototype(proto) {}
            virtual ~TooManyParameters() throw() {}

        protected:
            virtual const std::string causedBy() { return "Too Many arguments, prototype is " + prototype; }
        };
    }
}

#endif
