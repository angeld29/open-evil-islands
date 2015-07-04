#ifndef DRIVER_H
#define DRIVER_H

#include <string>
#include <vector>

#include "EIScriptClassesBase.hpp" // TODO is this acceptable?

namespace EIScript
{

    /** The Driver class brings together all components. It creates an instance of
     * the Parser and Scanner classes and connects them. Then the input stream is
     * fed into the scanner object and the parser gets it's token
     * sequence. Furthermore the driver object is available in the grammar rules as
     * a parameter. Therefore the driver class contains a reference to the
     * structure into which the parsed data is saved. */
    class Driver
    {
        friend class BisonParser;
    public:
        /** Pointer to the current lexer instance, this is used to connect the
         * parser to the scanner. */
        class FlexScanner* lexer = nullptr;

        /** Reference to the script context filled during parsing */
        class EIScriptContext* script_context;        
        /** Reference to the script functions implementation */
        EIScriptExecutor* script_executor;

        /// construct a new parser driver with default parameters
        Driver(EIScriptContext* script_context, EIScriptExecutor* script_executor);

        /// construct a new parser driver
        Driver(EIScriptContext* script_context, EIScriptExecutor* script_executor, bool trace_scanning, bool trace_parsing, bool standard_trace_parsing);

        ~Driver();

        /// enable debug output in the flex scanner
        bool trace_scanning;

        /// enable my debug output in the bison parser
        bool trace_parsing;

        /// enable standard debug output in the bison parser
        bool standard_trace_parsing;

        /// stream name (file or input stream) used for error messages.
        std::string streamname;

        /** Invoke the scanner and parser for a stream.
         * @param in	input stream
         * @param sname	stream name for error messages
         * @return		true if successfully parsed
         */
        bool parse_stream(std::istream& in, const std::string& sname = "stream input");

        /** Invoke the scanner and parser on a file. Use parse_stream with a
         * std::ifstream if detection of file reading errors is required.
         * @param filename	input file name
         * @return		true if successfully parsed
         */
        bool parse_file(const std::string& filename);

        // To demonstrate pure handling of parse errors, instead of
        // simply dumping them on the standard error output, we will pass
        // them to the driver using the following two member functions.

        /** Error handling with associated line number. This can be modified to
         * output the error e.g. to a dialog box. */
        void error(const class location& l, const std::string& m);

        /** General error handling. This can be modified to output the error
         * e.g. to a dialog box. */
        void error(const std::string& m);

    protected:
        void push_context(EIScriptContext* context);
        void pop_context();
    };
}

#endif // DRIVER_H
