#include "AIDirector.hpp"

namespace cursedearth
{

    AIDirector::AIDirector()
        : script_context(new EIScript::EIScriptContext())
        , script_functions(new EIScript::EIScriptFunctions(this))
        , script_executor(new EIScript::EIScriptExecutor(script_context, script_functions))
        , driver(script_context, script_executor, false, true, false)
    {
        std::memset(ai_diplomacy, 0, sizeof(ai_diplomacy[0][0]) * n_players * n_players);
    }

    AIDirector::~AIDirector()
    {
        delete script_executor;
        delete script_functions;
        delete script_context;
    }

    void AIDirector::advance(float elapsed){
        script_executor->advance(elapsed);
    }

    void AIDirector::GSSetVar(double player, std::string* variable, double value)
    {
    }

    void AIDirector::GSSetVarMax(double player, std::string* variable, double value)
    {
    }

    double AIDirector::GSGetVar(double player, std::string* variable)
    {
        return 1;
    }

    void AIDirector::GSDelVar(double player, std::string* variable)
    {
    }

    double AIDirector::GetDiplomacy(double player_one, double player_two)
    {
        int one = std::round(player_one);
        int two = std::round(player_two);
        if(one >= n_players || two >= n_players) {
            throw EIScript::Exception::InvalidAction(
            "Invalid player numbers: " + std::to_string(player_one) + ", " + std::to_string(player_two), tag
            );
        } else {
            return ai_diplomacy[one][two];
        }
    }

    void AIDirector::SetDiplomacy(double player_one, double player_two, double value)
    {
        int one = std::round(player_one);
        int two = std::round(player_two);
        short int val = std::round(value);
        if(val > 0) { // TODO not sure how EI behaves when given something other than -1, 0, 1, I decided to be permissive
            val = 1;
        } else if(val < 0) {
            val = -1;
        }
        if(one >= n_players || two >= n_players) {
            throw EIScript::Exception::InvalidAction(
                "Invalid player numbers: " +  std::to_string(player_one) + ", " +  std::to_string(player_two), tag
            );
        } else {
            ai_diplomacy[one][two] = val;
        }
    }

}
