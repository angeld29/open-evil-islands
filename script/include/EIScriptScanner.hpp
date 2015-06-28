#pragma once

// Only include FlexLexer.h if it hasn't been already included
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

// Override the interface for yylex since we namespaced it
#undef YY_DECL
#define YY_DECL int EIScript::FlexScanner::yylex(				\
	EIScript::BisonParser::semantic_type* yylval,		\
	EIScript::BisonParser::location_type* yylloc		\
    )

// Include Bison for types / tokens
#include "eiscript.tab.hpp"

namespace EIScript
{
class FlexScanner : public yyFlexLexer
{
public:

    FlexScanner(std::istream* arg_yyin = 0,
	    std::ostream* arg_yyout = 0);
    
    int yylex();
    
    virtual int yylex(
        EIScript::BisonParser::semantic_type* yylval,
        EIScript::BisonParser::location_type* yylloc
	);
    
    void set_debug(bool b)
    {
        yy_flex_debug = b;
    }

private:

};
}
