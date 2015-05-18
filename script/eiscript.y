%require "2.4.1"
%skeleton "lalr1.cc"
%defines
%define namespace "EIScript"
%define parser_class_name "BisonParser"
%parse-param { class Driver& driver }
%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.streamname;
};

//disable in release
%debug
/* verbose error messages */
%error-verbose

%code requires {
    #include <iostream>
    #include <string>
    #include "eiscript_base.h"
    #include "EIScriptContext.h"
}

%code {
    #include "EIScriptDriver.h"
    #include "EIScriptScanner.h"
    
    #undef yylex
    #define yylex driver.lexer->yylex
}

%union {
    Type tVal;
    float fVal;
    std::string* sVal;
}

%token <tVal> FLOAT STRING OBJECT GROUP
%token <fVal> FLOATNUMBER
%token <sVal> CHARACTER_STRING /* unfinished */ GLOBALVARS DECLARESCRIPT SCRIPT IF THEN WORLDSCRIPT IDENTIFIER FOR
%type  <sVal> ident
%type  <tVal> type

%%

eiscript  : globalVars declarations scripts worldscript { std::cout<<"Program accepted."; return 0; }
          ;

globalVars : GLOBALVARS '(' globalVarsDefs ')'
          ;

declarations : /* empty */
          | script_declaration
          | declarations script_declaration
          ;

scripts : /* empty */
          | script_implementation
          | scripts script_implementation
          ;

worldscript: WORLDSCRIPT '(' script_then_body ')' { std::cout<<"Worldscript."<<std::endl; }

globalVarsDefs : /* empty */ 
          | globalVarDef
          | globalVarsDefs ',' globalVarDef 
          ;

globalVarDef : ident ':' type { std::cout<<"Declared global variable "<<*$1<<" of type "<<$3<<std::endl; }
          ;
          
script_declaration : DECLARESCRIPT ident formal_params { std::cout<<"Declared script "<<*$2<<std::endl; }
          ;
     
script_implementation : SCRIPT ident '(' script_body ')' { std::cout<<"Implemented script "<<*$2<<std::endl; }
          ;
          
script_body : script_block
          | script_body script_block
          ;
        
script_block : script_if_block script_then_block
          ;
          
script_if_block : IF '(' if_conjunction ')'
          ;
          
if_conjunction : /* empty */
          | float_expression /* restrict to 0 or 1? */
          | if_conjunction float_expression
          ;
          
script_then_block : THEN '(' script_then_body ')'
          ;
          
script_then_body : /* empty */
          | function_call
          | assignment
          | for_block
          | script_then_body function_call
          | script_then_body assignment
          | script_then_body for_block
          ;

for_block : FOR '(' ident ',' ident ')' '(' script_then_body ')'

formal_params : '(' formal_parameter_list ')' 
          ;
          
formal_parameter_list : formal_parameter_list ',' formal_parameter
          | formal_parameter
          ;

formal_parameter : ident ':' type
          ;
          
params : '(' actual_parameter_list ')' 
          ;
          
actual_parameter_list : actual_parameter_list ',' actual_parameter
          | actual_parameter
          | /* empty */
          ;

actual_parameter : expression
          ;
          
expression : FLOATNUMBER
          | CHARACTER_STRING
          | ident
          | assignment
          | function_call /* check for non-void return type?*/
          ;
          
float_expression : FLOATNUMBER
          | ident /* check for float type?*/
          | function_call /* check for float return type?*/
          ;

assignment : ident '=' expression
          ;

function_call : ident params /* function or script */ /* validate? */
          ;
          
type : FLOAT  /*{ $$ = $1; }*/
          | STRING
          | OBJECT
          | GROUP
          ;
          
ident : IDENTIFIER { $$ = $1; }
          ;
          
%%

// We have to implement the error function
void EIScript::BisonParser::error(const EIScript::BisonParser::location_type &loc, const std::string &msg) {
	std::cerr << "Error: " << msg << std::endl;
	std::cerr << "Location: " << loc << std::endl;
}