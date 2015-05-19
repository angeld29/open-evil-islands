%{
    #include <iostream>
    #include <cstdlib>
    #include "eiscript.tab.hh"
    #include "EIScriptScanner.h"

    void commenteof();
    int line_no = 1;
%}

%option nodefault yyclass="FlexScanner" yywrap c++

%{
#define YY_USER_ACTION  yylloc->columns(yyleng);
%}

A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]

%%

%{
    // reset location
    yylloc->step();
%}

{F}{L}{O}{A}{T}                          { yylval->tVal = Type::Float; return(EIScript::BisonParser::token::FLOAT); }
{S}{T}{R}{I}{N}{G}                       { yylval->tVal = Type::String; return(EIScript::BisonParser::token::STRING); }
{O}{B}{J}{E}{C}{T}                       { yylval->tVal = Type::Object; return(EIScript::BisonParser::token::OBJECT); }
{G}{R}{O}{U}{P}                          { yylval->tVal = Type::Group; return(EIScript::BisonParser::token::GROUP); }
{G}{L}{O}{B}{A}{L}{V}{A}{R}{S}           { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::GLOBALVARS); }
{D}{E}{C}{L}{A}{R}{E}{S}{C}{R}{I}{P}{T}  { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::DECLARESCRIPT); }
{S}{C}{R}{I}{P}{T}                       { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::SCRIPT); }
{I}{F}                                   { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::IF); }
{T}{H}{E}{N}                             { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::THEN); }
{F}{O}{R}                                { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::FOR); }
{W}{O}{R}{L}{D}{S}{C}{R}{I}{P}{T}        { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::WORLDSCRIPT); }
[#a-zA-Z]([#_a-zA-Z0-9])*                { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::IDENTIFIER); }
-?[0-9][0-9]*|[0-9]+"."[0-9]+            { yylval->fVal = std::atof(yytext); return(EIScript::BisonParser::token::FLOATNUMBER); }
\"[^"]*\"                                { yylval->sVal = new std::string(yytext, yyleng); return(EIScript::BisonParser::token::CHARACTER_STRING); }

[=(){},:] {
            return *yytext;
        }
             

"//"  { 
          register int c;
          while ((c = yyinput()))
          {
              yylloc->step();
              if (c == '\n'){
                  yylloc->lines(1);
                  line_no++;
                  break;
              } else if (c == 0)
                  commenteof();
          }
         
      }

[ \f\t\r]+ {
    yylloc->step();
}

\n {
    line_no++;
    yylloc->lines(yyleng); 
    yylloc->step();
}

.     { 
          std::cerr<<"'"<<*yytext<<"' "<<"illegal character at line "<<line_no<<std::endl;
      }

%%

void commenteof()
{
    std::cerr<<"unexpected EOF inside comment at line"<<line_no<<std::endl;
    exit (1);
}

EIScript::FlexScanner::FlexScanner(std::istream* in,
		 std::ostream* out)
    : yyFlexLexer(in, out)
{
}


int EIScript::FlexScanner::yylex()
{
    std::cerr << "in FlexScanner::yylex() !" << std::endl;
    return 0;
}


int yyFlexLexer::yywrap()
{
    line_no = 1; //?
    return 1;
}