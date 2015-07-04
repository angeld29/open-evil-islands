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
    #include "EIScriptClasses.hpp"
    #include "EIScriptContext.hpp"
    #include "EIScriptExecutor.hpp"
}

%code {
    #include "EIScriptDriver.hpp"
    #include "EIScriptScanner.hpp"
    
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
%token <tVal> FLOAT STRING OBJECT GROUP
%token <fVal> FLOATNUMBER
%token <sVal> IDENTIFIER CHARACTER_STRING
%type  <tVal> type
%type  <identifierVal> ident
%type  <expressionVal> script_expression actual_parameter expression variable assignment function_call for_block
%type  <varDeclarationVal> globalVarDef formal_parameter
%type  <variableListVal> formal_params formal_parameter_list
%type  <scriptDeclarationVal> script_declaration
%type  <scriptBody> script_body
%type  <scriptBlock> script_block
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
                                                        ScriptDeclaration* worldscript = new ScriptDeclaration(new Identifier(new std::string("WorldScript")), nullptr); //yeek
                                                        worldscript->setScriptBody(new ScriptBody{new ScriptBlock{nullptr, $3}});
                                                        driver.script_context->setWorldscript(worldscript);
                                                        if(driver.trace_parsing) {
                                                            std::cout<<"Worldscript."<<std::endl;
                                                        } 
                                                    }
          ;

globalVarsDefs : /* empty */ 
          | globalVarDef                    { driver.script_context->addGlobalVariable($1); }
          | globalVarsDefs ',' globalVarDef { driver.script_context->addGlobalVariable($3); }
          ;

globalVarDef : ident ':' type               { 
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
                                                                if(driver.script_executor->functionDefined($2)) {
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
                                                driver.push_context(driver.script_context->extendedContext(scriptDeclaration->getArguments()));
                                            } 
                         script_body ')'    { 
                                                driver.pop_context();
                                                ScriptDeclaration* scriptDeclaration = driver.script_context->getScript($2);
                                                scriptDeclaration->setScriptBody($5);
                                                if(driver.trace_parsing) {
                                                    std::cout<<"Implemented script "<<*($2->name)<<std::endl; 
                                                }
                                            }
          ;
          
script_body : script_block                  { $$ = new ScriptBody{$1}; }
          | script_body script_block        { $$ = $1; $$->push_back($2); }
          ;
        
script_block : script_if_block script_then_block { $$ = new ScriptBlock{$1, $2}; }
          ;
          
script_if_block : IF '(' if_conjunction ')' { $$ = $3; }
          ;
          
if_conjunction : /* empty */                { $$ = new ExpressionList(); }
          | function_call                   { $$ = new ExpressionList{$1}; } /* check for float return type?*/ /* restrict to 0 or 1? */
          | if_conjunction function_call    { $$ = $1; $$->push_back($2); }
          ;
          
script_then_block : THEN '(' script_then_body ')' { $$ = $3; }
          ;
          
script_then_body : /* empty */              { $$ = new ExpressionList(); }
          | script_expression               { $$ = new ExpressionList{$1}; }
          | script_then_body script_expression { $$ = $1; $$->push_back($2); }
          ;
          
script_expression : function_call
          | assignment
          | for_block
          ;

for_block : FOR '(' ident ',' ident ')' '(' script_then_body ')' { $$ = nullptr; } /* NIY */

formal_params : '(' formal_parameter_list ')' { $$ = $2; }
          ;
          
formal_parameter_list : formal_parameter_list ',' formal_parameter { $$ = $1; $$->push_back($3); }
          | formal_parameter                { $$ = new VariableList{$1}; }
          | /* empty */                     { $$ = new VariableList(); }
          ;

formal_parameter : ident ':' type           { $$ = new VariableDeclaration($3, $1); }
          ;
          
params : '(' actual_parameter_list ')'      { $$ = $2; }
          ;
          
actual_parameter_list : actual_parameter_list ',' actual_parameter { $$ = $1; $$->push_back($3); }
          | actual_parameter                { $$ = new ExpressionList{$1}; }
          | /* empty */                     { $$ = new ExpressionList(); }
          ;

actual_parameter : expression
          ;
          
expression : FLOATNUMBER                    { $$ = new FloatValue($1); }
          | CHARACTER_STRING                { $$ = new StringValue($1); }
          | variable                        
          | function_call /* check for non-void return type?*/
          ;

variable : ident                            { 
                                                VariableDeclaration* decl = driver.script_context->getVariable($1);
                                                if(!decl) {
                                                    error(yylocation_stack_[0], std::string("Variable ") + *($1->name) + std::string(" is not defined."));
                                                    if(driver.trace_parsing){
                                                        driver.script_context->dumpVariables(std::cerr);
                                                    }
                                                    return 1;
                                                } else {
                                                    $$ = new VariableAccess($1, decl->type);
                                                }
                                            }
         ;

function_call : ident params                { 
                                                if(driver.script_executor->functionDefined($1)){
                                                    $$ = new FunctionCall($1, $2, Type::Void); /* WIP */ 
                                                } else if (driver.script_context->scriptDefined($1)){
                                                    $$ = new ScriptCall($1, $2); /* WIP */
                                                } else {
                                                    $$ = nullptr;
                                                    std::cerr<<"Unknown function: "<<*$1->name<<" was called"<<std::endl; /* WIP */
                                                }
                                            } /* WIP */ /* function or script */ /* validate? */
          ;
          
assignment : ident '=' expression           { $$ = new Assignment($1, $3); }
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