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
    double fVal;
    std::string* sVal;
    Identifier* identifierVal;
    Expression* expressionVal;
    VariableDeclaration* varDeclarationVal;
    VariableList* variableListVal;   
    ScriptDeclaration* scriptDeclarationVal;
    ScriptBlock* scriptBlock;
    ScriptBody* scriptBody;
    ExpressionList* expressionList;
}

%token GLOBALVARS DECLARESCRIPT SCRIPT IF THEN WORLDSCRIPT FOR
%token <tVal> FLOAT STRING OBJECT GROUP type
%token <fVal> FLOATNUMBER
%token <sVal> IDENTIFIER CHARACTER_STRING
%type  <identifierVal> ident
%type  <expressionVal> script_expression actual_parameter expression assignment function_call 
%type  <varDeclarationVal> globalVarDef formal_parameter
%type  <variableListVal> formal_params formal_parameter_list
%type  <scriptDeclarationVal> script_declaration
%type  <scriptBlock> script_block
%type  <scriptBody> script_body
%type  <expressionList> script_if_block if_conjunction script_then_block script_then_body params actual_parameter_list

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
          | globalVarDef                    { driver.script_context->addGlobalVariable($1); }
          | globalVarsDefs ',' globalVarDef { driver.script_context->addGlobalVariable($3); }
          ;

 globalVarDef : ident ':' type              { 
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
                                                scriptDeclaration->setScriptBody($5); //is this valid, or should I look it up again?
                                                if(driver.trace_parsing) {
                                                    std::cout<<"Implemented script "<<*($2->name)<<std::endl; 
                                                }
                                            }
          ;
          
script_body : script_block                  { $$ = new ScriptBody{$1}; }
          | script_body script_block        { $$ = $1; $$->push_back($2); }
          ;
        
script_block : script_if_block script_then_block { $$ = new ScriptBlock($1, $2); }
          ;
          
script_if_block : IF '(' if_conjunction ')' { $$ = $3 }
          ;
          
if_conjunction : /* empty */
          | function_call                   { $$ = new ExpressionList{$1}; } /* check for float return type?*/ /* restrict to 0 or 1? */
          | if_conjunction function_call    { $$ = new $1; $$->push_back($2); }
          ;
          
script_then_block : THEN '(' script_then_body ')' { $$ = $3 }
          ;
          
script_then_body : /* empty */
          | script_expression               { $$ = new ExpressionList{$1}; }
          | script_then_body script_expression { $$ = $1; $$->push_back($2); }
          ;
          
script_expression : function_call
          | assignment
          | for_block
          ;

for_block : FOR '(' ident ',' ident ')' '(' script_then_body ')'

formal_params : '(' formal_parameter_list ')' { $$ = $2; }
          ;
          
formal_parameter_list : formal_parameter_list ',' formal_parameter { $$ = $1; $$->push_back($3); }
          | formal_parameter                { $$ = new VariableList{$1}; }
          | /* empty */                     { $$ = nullptr; }
          ;

formal_parameter : ident ':' type           { $$ = new VariableDeclaration($3, $1); }
          ;
          
params : '(' actual_parameter_list ')'      { $$ = $2; }
          ;
          
actual_parameter_list : actual_parameter_list ',' actual_parameter { $$ = $1; $$->push_back($3); }
          | actual_parameter                { $$ = new ExpressionList{$1}; }
          | /* empty */
          ;

actual_parameter : expression
          ;
          
variable : ident                            { 
                                                if(!driver.script_context->variableDefined($1)) {
                                                    error(yylocation_stack_[0], std::string("Variable ") + *($1->name) + std::string(" is not defined."));
                                                    if(driver.trace_parsing){
                                                        driver.script_context->dumpVariables(std::cerr);
                                                    }
                                                    return 1;
                                                }
                                            }
         ;

assignment : ident '=' expression           { $$ = new Assignment($1, $3); }
          ;
          
expression : FLOATNUMBER                    { $$ = new FloatValue($1); }
          | CHARACTER_STRING                { $$ = new StringValue($1); }
          | variable 
          | function_call /* check for non-void return type?*/
          ;

function_call : ident params /* function or script */ /* validate? */
          ;
          
type : FLOAT
          | STRING
          | OBJECT
          | GROUP
          ;
          
ident : IDENTIFIER                          { $$ = new Identifier($1); }
          ;
          
%%

// We have to implement the error function
void EIScript::BisonParser::error(const EIScript::BisonParser::location_type &loc, const std::string &msg) {
	std::cerr << "Error: " << msg << std::endl;
	std::cerr << "Location: " << loc << std::endl;
}