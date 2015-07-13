#include "AIDirector.hpp"
#include "logging.hpp"

#include <algorithm>

namespace cursedearth
{

    AIDirector::AIDirector()
        : script_context(new EIScript::EIScriptContext())
        , script_functions(new EIScript::EIScriptFunctions(this))
        , script_executor(new EIScript::EIScriptExecutor(script_context, script_functions))
        , driver(new EIScript::Driver(script_context, script_executor, false, true, false))
        , player(new Player())
    {
        std::memset(ai_diplomacy, 0, sizeof(ai_diplomacy[0][0]) * n_players * n_players);
        gs_vars = new std::map<std::string, double>();
        map_objects = new std::map<uint32_t, mob_object*>();
    }

    AIDirector::~AIDirector()
    {
        delete gs_vars;
        delete map_objects;
        delete script_executor;
        delete script_functions;
        delete script_context;
        delete driver;
    }

    void AIDirector::acceptMob(mob_file* mob_file)
    {
        for(auto& object : *(mob_file->get_objects())){
            (*map_objects)[object->id] = object;
        }
    }

    bool AIDirector::parseScript(ce_string* script, ce_string* mob_name){
        ce_logging_debug("Parsing mob %s", mob_name);
        std::string name(mob_name->str ? mob_name->str : "unknown mob" );
        std::istringstream script_stream(script->str);
        return driver->parse_stream(script_stream, name);
    }

    bool AIDirector::parseStream(std::istream& in, const std::string& sname)
    {
        return driver->parse_stream(in, sname);
    }

    bool AIDirector::parseFile(const std::string& filename)
    {
        return driver->parse_file(filename);
    }

    void AIDirector::startExecution()
    {
        if(script_executor->queueIsEmpty()) {
            script_executor->callScript(script_context->getWorldscript()->getId(), nullptr);
        } else {
            ce_logging_error("Tried to initiate script execution when the script queue was already non-empty.");
        }
    }

    void AIDirector::advance(float elapsed){
        script_executor->advance(elapsed);
    }

    void AIDirector::GSSetVar(double player, std::string* variable, double value)
    {
        (*gs_vars)[std::to_string(player) + *variable] = value;
    }

    void AIDirector::GSSetVarMax(double player, std::string* variable, double value)
    {
    }

    double AIDirector::GSGetVar(double player, std::string* variable)
    {
        return (*gs_vars)[std::to_string(player) + *variable];
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

    double AIDirector::GetLeverState(EIScript::ScriptObject* object)
    {
        mob_lever* lever = dynamic_cast<mob_lever*>(object);
        if(!lever) {
            throw EIScript::Exception::InvalidAction("Not a lever.", tag); // TODO proper
        } else {
            return lever->get_state();
        }
    }

    EIScript::ScriptObject* AIDirector::GetLeader()
    {
        return player;
    }

    EIScript::ScriptObject* AIDirector::GetObject(double id)
    {
        uint32_t uint_id = id; // TODO proper
        return (*map_objects)[uint_id];
    }

    EIScript::ScriptObject* AIDirector::GetObjectById(std::string* id)
    {
       uint32_t uint_id = string2uint32(id);
       return (*map_objects)[uint_id];
    }

    EIScript::ScriptObject* AIDirector::GetObjectByName(std::string* name)
    {
        auto entry = std::find_if(
                    map_objects->begin(),
                    map_objects->end(),
                    [name](const std::pair<const uint32_t, cursedearth::mob_object*>& arg)
                    {
                        return strcmp(arg.second->name->str, name->c_str()) == 0;
                    }
        );
        if(entry != map_objects->end()){
            return entry->second;
        } else {
            throw EIScript::Exception::InvalidParameter("Could not find object with name: " + *name, tag);
        }
    }

    void AIDirector::setMyTraceParsing(bool trace_parsing)
    {
        driver->trace_parsing = trace_parsing;
    }

    void AIDirector::setTraceParsing(bool trace_parsing)
    {
        driver->standard_trace_parsing = trace_parsing;
    }

    void AIDirector::setTraceScanning(bool trace_scanning)
    {
        driver->trace_scanning = trace_scanning;
    }

}
