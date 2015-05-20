#include <fstream>
#include <sstream>

#include "EIScriptDriver.h"
#include "EIScriptScanner.h"
#include "EIScriptParser.h"

namespace EIScript
{

    Driver::Driver(class EIScriptContext* _script_context)
        : script_context(_script_context)
        , trace_scanning(false)
        , trace_parsing(false)
        , standard_trace_parsing(false)
    {
    }

    Driver::Driver(class EIScriptContext* _script_context, bool trace_scanning, bool trace_parsing, bool standard_trace_parsing)
        : script_context(_script_context)
        , trace_scanning(trace_scanning)
        , trace_parsing(trace_parsing)
        , standard_trace_parsing(standard_trace_parsing)
    {
    }

    bool Driver::parse_stream(std::istream& in, const std::string& sname)
    {
        streamname = sname;

        FlexScanner scanner(&in);
        scanner.set_debug(trace_scanning);
        this->lexer = &scanner;

        Parser parser(*this);
        if(standard_trace_parsing) {
            parser.set_debug_level(trace_parsing);
            parser.set_debug_stream(std::cerr);
        }
        return (parser.parse() == 0);
    }

    bool Driver::parse_file(const std::string& filename)
    {
        std::ifstream in(filename.c_str());
        if(!in.good())
            return false;
        return parse_stream(in, filename);
    }

    //TODO perhaps move error reporting here?
    //TODO add a trace function with customizable output stream?
    void Driver::error(const class location& l, const std::string& m)
    {
        std::cerr << l << ": " << m << std::endl;
    }

    void Driver::error(const std::string& m)
    {
        std::cerr << m << std::endl;
    }

    void Driver::push_context(EIScriptContext* context)
    {
        if(context->getParentContext() != script_context) {
            std::cerr << "Tried to replace parent context." << std::endl;
        } else {
            script_context = context;
        }
    }

    void Driver::pop_context()
    {
        if(script_context->getParentContext() == nullptr) {
            std::cerr << "Tried to pop parent context." << std::endl;
        } else {
            EIScriptContext* previous = script_context;
            script_context = script_context->getParentContext();
            delete previous;
        }
    }

}
