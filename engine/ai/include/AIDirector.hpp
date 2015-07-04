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
#include "mobfile.hpp"
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

        bool parseScript(ce_string* script, ce_string* mob_name);
        bool parseStream(std::istream& in, const std::string& sname = "stream input");
        bool parseFile(const std::string& filename);

        void startExecution();
        void advance(float elapsed);

        void GSSetVar(double player, std::string* variable, double value);
        void GSSetVarMax(double player, std::string* variable, double value);
        double GSGetVar(double player, std::string* variable);
        void GSDelVar(double player, std::string* variable);
        double GetDiplomacy(double player_one, double player_two);
        void SetDiplomacy(double player_one, double player_two, double value);

        void setMyTraceParsing(bool trace_parsing);
        void setTraceParsing(bool trace_parsing);
        void setTraceScanning(bool trace_scanning);

    protected:
        const int n_players = 32;
        boost::unordered_map<std::string, double> gs_vars; // Why gs_vars? GSSetVar? What is GS? Who is the Milkman?
        short int ai_diplomacy[32][32]; // because of dumb
        std::string tag = "[AIDirector]";

        EIScript::EIScriptContext* script_context;
        EIScript::EIScriptFunctionsBase* script_functions;
        EIScript::EIScriptExecutor* script_executor;
        EIScript::Driver* driver;
    };

    typedef std::shared_ptr<AIDirector> AIDirectorPointerType;
    typedef std::shared_ptr<const AIDirector> AIDirectorConstPointerType;

}

#endif // AI_DIRECTOR
