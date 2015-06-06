#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <string>
#include <boost/unordered_map.hpp>
#include "ScriptAccessible.hh"

namespace cursedearth
{

    class AIDirector : public EIScript::ScriptAccessible
    {
    public:
        AIDirector() {
        }
        ~AIDirector() {
        }

        void GSSetVar(double player, std::string* variable, double value);
        void GSSetVarMax(double player, std::string* variable, double value);
        double GSGetVar(double player, std::string* variable);
        void GSDelVar(double player, std::string* variable);

    protected:
        boost::unordered_map<std::string, double> gs_vars; // Why gs_vars? GSSetVar? What is GS?
    };

}

#endif // AI_DIRECTOR
