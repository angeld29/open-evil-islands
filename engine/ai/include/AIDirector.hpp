#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <cstring>
#include <string>
#include <cmath>
#include <boost/unordered_map.hpp>

#include "utility.hpp"
#include "ScriptAccessible.hpp"
#include "scriptexception.hpp"

namespace cursedearth
{

    class AIDirector : public EIScript::ScriptAccessible
    {
    public:
        AIDirector();
        ~AIDirector();

        void advance(float elapsed);

        void GSSetVar(double player, std::string* variable, double value);
        void GSSetVarMax(double player, std::string* variable, double value);
        double GSGetVar(double player, std::string* variable);
        void GSDelVar(double player, std::string* variable);
        double GetDiplomacy(double player_one, double player_two);
        void SetDiplomacy(double player_one, double player_two, double value);

    protected:
        const int n_players = 32;
        boost::unordered_map<std::string, double> gs_vars; // Why gs_vars? GSSetVar? What is GS? Who is the Milkman?
        short int ai_diplomacy[32][32]; // because of dumb
        std::string tag = "[AIDirector]";

        EIScript::EIScriptContext* script_context;
        EIScript::EIScriptFunctions* script_functions;
        EIScript::EIScriptExecutor* script_executor;
        EIScript::Driver& driver;
    };

}

#endif // AI_DIRECTOR
