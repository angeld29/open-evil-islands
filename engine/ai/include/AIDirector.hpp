#ifndef AI_DIRECTOR
#define AI_DIRECTOR

#include <cstring>
#include <string>
#include <sstream>
#include <cmath>
#include <memory>
#include <boost/unordered_map.hpp>

#include "utility.hpp"
#include "string.hpp"
#include "ScriptAccessible.hpp"
#include "EIScriptContext.hpp"
#include "EIScriptFunctions.hpp"
#include "EIScriptExecutor.hpp"
#include "EIScriptDriver.hpp"
#include "scriptexception.hpp"

namespace cursedearth
{

    class AIDirector : public EIScript::ScriptAccessible
    {
    public:
        AIDirector();
        ~AIDirector();

        void parseScript(ce_string* script, std::string& mob_name);

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
        EIScript::Driver* driver;
    };

    typedef std::shared_ptr<AIDirector> AIDirectorPointerType;
    typedef std::shared_ptr<const AIDirector> AIDirectorConstPointerType;

}

#endif // AI_DIRECTOR
