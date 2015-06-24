#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <cstring>
#include <string>
#include <cmath>
#include <boost/unordered_map.hpp>

#include "utility.hpp"
#include "ScriptAccessible.hh"
#include "exception.hh"

namespace cursedearth
{

    class AIDirector : public EIScript::ScriptAccessible
    {
    public:
        AIDirector();
        ~AIDirector();

        void GSSetVar(double player, std::string* variable, double value);
        void GSSetVarMax(double player, std::string* variable, double value);
        double GSGetVar(double player, std::string* variable);
        void GSDelVar(double player, std::string* variable);
        double GetDiplomacy(double player_one, double player_two);
        void SetDiplomacy(double player_one, double player_two, double value);

    protected:
        const int n_players = 32;
        boost::unordered_map<std::string, double> gs_vars; // Why gs_vars? GSSetVar? What is GS?
        short int ai_diplomacy[32][32]; // because of dumb
        std::string tag = "[AIDirector]";
    };

}

#endif // AI_DIRECTOR
