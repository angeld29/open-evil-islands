#ifndef EI_SCRIPT_FUNCTIONS
#define EI_SCRIPT_FUNCTIONS

#include <string>
#include <boost/unordered_map.hpp>
#include "EIScriptClassesBase.h"
#include "EIScriptFunctionsBase.h"
#include "function.hh"

namespace EIScript
{

    struct CaseInsensitiveHash {
        std::size_t operator()(const std::string& Keyval) const {

            std::size_t hash = 5381;
            std::for_each(Keyval.begin() , Keyval.end() , [&](char c) {
                hash += ((hash << 5) + hash) + std::tolower(c);
            });
            return hash;
        }
    };

    struct CaseInsensitiveEqual {
        bool operator()(const std::string& Left, const std::string& Right) const {
            return Left.size() == Right.size()
                   && std::equal(Left.begin(), Left.end(), Right.begin(),
            [](char a , char b) {
                return std::tolower(a) == std::tolower(b);
            }
                                );
        }
    };


    class EIScriptFunctions : public cursedearth::EIScriptFunctionsBase<EIScriptFunctions>
    {
        typedef typename Function::AbstractFunction FunctionType;
        typedef typename cursedearth::AIDirector<EIScriptFunctions> DirectorType;
    public:
        EIScriptFunctions(DirectorType* ai_director);

        ~EIScriptFunctions();

        virtual Expression* call(std::string* function_name, ExpressionList& arguments);

        virtual bool functionDefined(std::string* function_name);
        virtual Type getFunctionType(std::string* function_name);
        virtual void dumpFunctions(std::ostream& str);

    protected:
        boost::unordered_map<std::string, FunctionType*, CaseInsensitiveHash, CaseInsensitiveEqual> functions;

        void initFunctions();
    };

}

#endif // EI_SCRIPT_FUNCTIONS
