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
    #include "EIScriptClasses.h"
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
    Identifier* identifierVal;
    VariableDeclaration* varDeclarationVal;
    VariableList* variableListVal;   
    ScriptDeclaration* scriptDeclarationVal;
}

%token <tVal> FLOAT STRING OBJECT GROUP
%token <fVal> FLOATNUMBER
%token <sVal> CHARACTER_STRING /* unfinished */ GLOBALVARS DECLARESCRIPT SCRIPT IF THEN WORLDSCRIPT IDENTIFIER FOR
%type  <tVal> type
%type  <identifierVal> ident
%type  <varDeclarationVal> globalVarDef formal_parameter
%type  <variableListVal> formal_params formal_parameter_list
%type  <scriptDeclarationVal> script_declaration

%%

eiscript  : globalVars declarations scripts worldscript { std::cout<<"Program accepted."; return 0; }
          ;

globalVars : GLOBALVARS '(' globalVarsDefs ')'
          ;

declarations : /* empty */
          | script_declaration              { driver.script_context->addScript($1); }
          | declarations script_declaration { driver.script_context->addScript($2); }
          ;

scripts : /* empty */
          | script_implementation           
          | scripts script_implementation   
          ;

worldscript: WORLDSCRIPT '(' script_then_body ')'   {
                                                        if(driver.trace_parsing) {
                                                            std::cout<<"Worldscript."<<std::endl;
                                                        } 
                                                    }

globalVarsDefs : /* empty */ 
          | globalVarDef                    { driver.script_context->addVariable($1); }
          | globalVarsDefs ',' globalVarDef { driver.script_context->addVariable($3); }
          ;

globalVarDef : ident ':' type   { 
                                    if(driver.script_context->variableDefined($1)) {
                                        error(yylocation_stack_[0], std::string("Duplicate variable definition: ") + *($1->name));
                                        return 1;
                                    }
                                    $$ = new VariableDeclaration($3, $1);
                                    if(driver.trace_parsing){
                                        std::cout<<"Declared global variable "<<*($1->name)<<" of type "<<$3<<std::endl;
                                    }
                                }
          ;
          
script_declaration : DECLARESCRIPT ident formal_params      { 
                                                                if(driver.script_context->scriptDefined($2)) {
                                                                    error(yylocation_stack_[0], std::string("Duplicate script definition: ") + *($2->name));
                                                                    return 1;
                                                                }
                                                                $$ = new ScriptDeclaration($2, $3);
                                                                if(driver.script_context->functionDefined($2)) {
                                                                    error(yylocation_stack_[0], std::string("Possibly overshadowing  definition: script ") + *($2->name));
                                                                }                          
                                                                if(driver.trace_parsing) {
                                                                    std::cout<<"Declared script "<<*($2->name)<<std::endl;
                                                                }
                                                            }
          ;
     
script_implementation : SCRIPT ident '('    { 
                                                ScriptDeclaration* scriptDeclaration = driver.script_context->getScript($2);
                                                if(!scriptDeclaration) {
                                                    error(yylocation_stack_[0], std::string("Found implementation for an undefined script: ") + *($2->name));
                                                    return 1;
                                                }
                                                driver.push_context(driver.script_context->extendedContext(scriptDeclaration->arguments));
                                            } 
                         script_body ')'    { 
                                                driver.pop_context();
                                                if(driver.trace_parsing) {
                                                    std::cout<<"Implemented script "<<*($2->name)<<std::endl; 
                                                }
                                            }
          ;
          
script_body : script_block
          | script_body script_block
          ;
        
script_block : script_if_block script_then_block
          ;
          
script_if_block : IF '(' if_conjunction ')'
          ;
          
if_conjunction : /* empty */
          | function_call  /* check for float return type?*/ /* restrict to 0 or 1? */
          | if_conjunction function_call
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

formal_params : '(' formal_parameter_list ')' { $$ = $2; }
          ;
          
formal_parameter_list : formal_parameter_list ',' formal_parameter { $$ = $1; $$->push_back($3); }
          | formal_parameter { $$ = new VariableList{$1}; }
          | /* empty */ { $$ = nullptr; }
          ;

formal_parameter : ident ':' type { $$ = new VariableDeclaration($3, $1); }
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
          | variable 
          | assignment
          | function_call /* check for non-void return type?*/
          ;

variable : ident    { 
                        if(!driver.script_context->variableDefined($1)) {
                            error(yylocation_stack_[0], std::string("Variable ") + *($1->name) + std::string(" is not defined."));
                            if(driver.trace_parsing){
                                driver.script_context->dumpVariables(std::cerr);
                            }
                            return 1;
                        }
                    }
         ;

assignment : ident '=' expression
          ;

function_call : ident params /* function or script */ /* validate? */
          ;
          
type : FLOAT
          | STRING
          | OBJECT
          | GROUP
          ;
          
ident : IDENTIFIER { $$ = new Identifier($1); }
          ;
          
%%

// We have to implement the error function
void EIScript::BisonParser::error(const EIScript::BisonParser::location_type &loc, const std::string &msg) {
	std::cerr << "Error: " << msg << std::endl;
	std::cerr << "Location: " << loc << std::endl;
}