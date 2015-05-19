#include <fstream>
#include <sstream>

#include "EIScriptDriver.h"
#include "EIScriptScanner.h"
#include "EIScriptParser.h"

namespace EIScript
{

    Driver::Driver(class EIScriptContext& _script_context)
        : trace_scanning(false)
        , trace_parsing(false)
        , script_context(_script_context)
    {
    }

    bool Driver::parse_stream(std::istream& in, const std::string& sname)
    {
        streamname = sname;

        FlexScanner scanner(&in);
        scanner.set_debug(trace_scanning);
        this->lexer = &scanner;

        Parser parser(*this);
        //parser.set_debug_level(trace_parsing);
        return (parser.parse() == 0);
    }

    bool Driver::parse_file(const std::string& filename)
    {
        std::ifstream in(filename.c_str());
        if(!in.good())
            return false;
        return parse_stream(in, filename);
    }

    void Driver::error(const class location& l, const std::string& m)
    {
        std::cerr << l << ": " << m << std::endl;
    }

    void Driver::error(const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}
