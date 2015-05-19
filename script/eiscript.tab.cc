/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* First part of user declarations.  */

/* Line 279 of lalr1.cc  */
#line 38 "eiscript.tab.cc"


#include "eiscript.tab.hh"

/* User implementation prologue.  */

/* Line 285 of lalr1.cc  */
#line 46 "eiscript.tab.cc"
/* Unqualified %code blocks.  */
/* Line 286 of lalr1.cc  */
#line 26 "eiscript.y"

    #include "EIScriptDriver.h"
    #include "EIScriptScanner.h"
    
    #undef yylex
    #define yylex driver.lexer->yylex


/* Line 286 of lalr1.cc  */
#line 59 "eiscript.tab.cc"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location) YYUSE(Type)
# define YY_REDUCE_PRINT(Rule)        static_cast<void>(0)
# define YY_STACK_PRINT()             static_cast<void>(0)

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

/* Line 353 of lalr1.cc  */
#line 4 "eiscript.y"
namespace EIScript {
/* Line 353 of lalr1.cc  */
#line 155 "eiscript.tab.cc"

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  BisonParser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  BisonParser::BisonParser (class Driver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {
  }

  BisonParser::~BisonParser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  BisonParser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  BisonParser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  BisonParser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
  
	default:
	  break;
      }
  }

  void
  BisonParser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  BisonParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  BisonParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  BisonParser::debug_level_type
  BisonParser::debug_level () const
  {
    return yydebug_;
  }

  void
  BisonParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline bool
  BisonParser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  BisonParser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  BisonParser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    // State.
    int yyn;
    int yylen = 0;
    int yystate = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    static semantic_type yyval_default;
    semantic_type yylval = yyval_default;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


/* User initialization code.  */
/* Line 545 of lalr1.cc  */
#line 9 "eiscript.y"
{
  // Initialize the initial location.
  yylloc.begin.filename = yylloc.end.filename = &driver.streamname;
}
/* Line 545 of lalr1.cc  */
#line 356 "eiscript.tab.cc"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
        YYCDEBUG << "Reading a token: ";
        yychar = yylex (&yylval, &yylloc);
      }

    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    // Compute the default @$.
    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }

    // Perform the reduction.
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
          case 2:
/* Line 670 of lalr1.cc  */
#line 56 "eiscript.y"
    { std::cout<<"Program accepted."; return 0; }
    break;

  case 5:
/* Line 670 of lalr1.cc  */
#line 63 "eiscript.y"
    { driver.script_context->addScript((yysemantic_stack_[(1) - (1)].funcDeclarationVal)); }
    break;

  case 6:
/* Line 670 of lalr1.cc  */
#line 64 "eiscript.y"
    { driver.script_context->addScript((yysemantic_stack_[(2) - (2)].funcDeclarationVal)); }
    break;

  case 10:
/* Line 670 of lalr1.cc  */
#line 72 "eiscript.y"
    {
                                                        if(driver.trace_parsing){
                                                            std::cout<<"Worldscript."<<std::endl;
                                                        } 
                                                    }
    break;

  case 12:
/* Line 670 of lalr1.cc  */
#line 79 "eiscript.y"
    { driver.script_context->addVariable((yysemantic_stack_[(1) - (1)].varDeclarationVal)); }
    break;

  case 13:
/* Line 670 of lalr1.cc  */
#line 80 "eiscript.y"
    { driver.script_context->addVariable((yysemantic_stack_[(3) - (3)].varDeclarationVal)); }
    break;

  case 14:
/* Line 670 of lalr1.cc  */
#line 83 "eiscript.y"
    { 
                                    if(driver.script_context->variableDefined((yysemantic_stack_[(3) - (1)].identifierVal))){
                                        error(yylocation_stack_[0], std::string("Duplicate variable definition: ") + *((yysemantic_stack_[(3) - (1)].identifierVal)->name));
                                        return 1;
                                    }
                                    (yyval.varDeclarationVal) = new VariableDeclaration((yysemantic_stack_[(3) - (3)].tVal), (yysemantic_stack_[(3) - (1)].identifierVal));
                                    if(driver.trace_parsing){
                                        std::cout<<"Declared global variable "<<*((yysemantic_stack_[(3) - (1)].identifierVal)->name)<<" of type "<<(yysemantic_stack_[(3) - (3)].tVal)<<std::endl;
                                    }
                                }
    break;

  case 15:
/* Line 670 of lalr1.cc  */
#line 95 "eiscript.y"
    { 
                                                                if(driver.script_context->scriptDefined((yysemantic_stack_[(3) - (2)].identifierVal))){
                                                                    error(yylocation_stack_[0], std::string("Duplicate script definition: ") + *((yysemantic_stack_[(3) - (2)].identifierVal)->name));
                                                                    return 1;
                                                                }
                                                                (yyval.funcDeclarationVal) = new FunctionDeclaration(Type::None, (yysemantic_stack_[(3) - (2)].identifierVal), (yysemantic_stack_[(3) - (3)].variableListVal));
                                                                if(driver.script_context->functionDefined((yysemantic_stack_[(3) - (2)].identifierVal))){
                                                                    error(yylocation_stack_[0], std::string("Possibly overshadowing  definition: script ") + *((yysemantic_stack_[(3) - (2)].identifierVal)->name));
                                                                }                          
                                                                if(driver.trace_parsing){
                                                                    std::cout<<"Declared script "<<*((yysemantic_stack_[(3) - (2)].identifierVal)->name)<<std::endl;
                                                                }
                                                            }
    break;

  case 16:
/* Line 670 of lalr1.cc  */
#line 110 "eiscript.y"
    { 
                                                                if(driver.trace_parsing){
                                                                    std::cout<<"Implemented script "<<*((yysemantic_stack_[(5) - (2)].identifierVal)->name)<<std::endl; 
                                                                }
                                                            }
    break;

  case 33:
/* Line 670 of lalr1.cc  */
#line 146 "eiscript.y"
    { (yyval.variableListVal) = (yysemantic_stack_[(3) - (2)].variableListVal); }
    break;

  case 34:
/* Line 670 of lalr1.cc  */
#line 149 "eiscript.y"
    { (yyval.variableListVal) = (yysemantic_stack_[(3) - (1)].variableListVal); (yyval.variableListVal)->push_back((yysemantic_stack_[(3) - (3)].varDeclarationVal)); }
    break;

  case 35:
/* Line 670 of lalr1.cc  */
#line 150 "eiscript.y"
    { (yyval.variableListVal) = new VariableList{(yysemantic_stack_[(1) - (1)].varDeclarationVal)}; }
    break;

  case 36:
/* Line 670 of lalr1.cc  */
#line 151 "eiscript.y"
    { (yyval.variableListVal) = nullptr; }
    break;

  case 37:
/* Line 670 of lalr1.cc  */
#line 154 "eiscript.y"
    { (yyval.varDeclarationVal) = new VariableDeclaration((yysemantic_stack_[(3) - (3)].tVal), (yysemantic_stack_[(3) - (1)].identifierVal)); }
    break;

  case 48:
/* Line 670 of lalr1.cc  */
#line 175 "eiscript.y"
    { 
                        if(!driver.script_context->variableDefined((yysemantic_stack_[(1) - (1)].identifierVal))){
                            error(yylocation_stack_[0], std::string("Variable ") + *((yysemantic_stack_[(1) - (1)].identifierVal)->name) + std::string(" is not defined."));
                            if(driver.trace_parsing){
                                driver.script_context->dumpVariables(std::cerr);
                            }
                            return 1;
                        }
                    }
    break;

  case 55:
/* Line 670 of lalr1.cc  */
#line 198 "eiscript.y"
    { (yyval.identifierVal) = new Identifier((yysemantic_stack_[(1) - (1)].sVal)); }
    break;


/* Line 670 of lalr1.cc  */
#line 609 "eiscript.tab.cc"
      default:
        break;
      }

    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */
        if (yychar <= yyeof_)
          {
            /* Return failure if at end of input.  */
            if (yychar == yyeof_)
              YYABORT;
          }
        else
          {
            yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
            yychar = yyempty_;
          }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (!yy_pact_value_is_default_ (yyn))
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	  YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystate_stack_.height ())
      {
        yydestruct_ ("Cleanup: popping",
                     yystos_[yystate_stack_[0]],
                     &yysemantic_stack_[0],
                     &yylocation_stack_[0]);
        yypop_ ();
      }

    return yyresult;
    }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (yychar != yyempty_)
          {
            /* Make sure we have latest lookahead translation.  See
               comments at user semantic actions for why this is
               necessary.  */
            yytoken = yytranslate_ (yychar);
            yydestruct_ (YY_NULL, yytoken, &yylval, &yylloc);
          }

        while (1 < yystate_stack_.height ())
          {
            yydestruct_ (YY_NULL,
                         yystos_[yystate_stack_[0]],
                         &yysemantic_stack_[0],
                         &yylocation_stack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  // Generate an error message.
  std::string
  BisonParser::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULL;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char BisonParser::yypact_ninf_ = -67;
  const signed char
  BisonParser::yypact_[] =
  {
        -5,    -8,    14,     6,    18,   -67,    18,    55,   -67,   -67,
      49,   -67,    20,    27,    18,    39,   -67,   -67,   -67,    18,
      23,    18,   -67,    29,    31,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,    51,   -67,    37,    61,    56,   -67,    18,
      23,    60,    12,   -67,    50,    62,    16,   -67,   -67,   -67,
     -10,   -67,   -67,    18,   -67,   -67,    64,   -67,    18,   -67,
     -67,   -67,   -67,     5,     5,   -67,    46,   -67,    66,    56,
      59,   -67,   -67,    57,   -67,   -67,   -67,   -67,   -67,   -10,
     -67,   -67,   -67,    21,    18,   -67,     5,   -67,    67,   -67,
      69,    56,    40,   -67
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  BisonParser::yydefact_[] =
  {
         0,     0,     0,     4,    11,     1,     0,     7,     5,    55,
       0,    12,     0,     0,     0,     0,     6,     8,     3,     0,
       0,    36,    15,     0,     0,     2,     9,    13,    51,    52,
      53,    54,    14,     0,    35,     0,     0,    25,    33,     0,
       0,     0,     0,    17,     0,     0,     0,    28,    27,    26,
       0,    34,    37,    21,    16,    18,     0,    19,     0,    10,
      31,    30,    29,    41,     0,    50,     0,    22,     0,    25,
       0,    43,    44,     0,    40,    42,    45,    46,    47,    48,
      49,    20,    23,     0,     0,    38,     0,    24,     0,    39,
       0,    25,     0,    32
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  BisonParser::yypgoto_[] =
  {
       -67,   -67,   -67,   -67,   -67,   -67,   -67,    65,    80,    73,
     -67,    32,   -67,   -67,   -67,   -66,   -40,   -67,   -67,    52,
     -67,   -67,     3,    26,   -67,   -41,   -45,    53,    -4
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  BisonParser::yydefgoto_[] =
  {
        -1,     2,     3,     7,    15,    25,    10,    11,     8,    17,
      42,    43,    44,    66,    57,    46,    47,    22,    33,    34,
      65,    73,    74,    75,    76,    48,    49,    32,    50
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char BisonParser::yytable_ninf_ = -1;
  const unsigned char
  BisonParser::yytable_[] =
  {
        12,    62,    13,    83,     1,    61,    60,    63,    67,     4,
      23,    64,    71,    72,     5,    12,     6,    35,    78,    78,
       9,    82,    77,    77,    41,    92,    28,    29,    30,    31,
      54,     9,    45,     9,    59,    35,     9,    45,    62,    87,
      20,    78,    61,    60,    21,    77,    36,    62,    37,    68,
      14,    61,    60,    24,    70,     9,    45,    40,    93,    79,
      79,     9,    68,    56,    81,     6,    14,    18,    19,    38,
      39,     9,    45,    41,    55,    85,    86,    53,    84,    58,
      88,    69,    79,    63,    27,    90,    91,    16,    26,    89,
      80,    51,     0,    52
  };

  /* YYCHECK.  */
  const signed char
  BisonParser::yycheck_[] =
  {
         4,    46,     6,    69,     9,    46,    46,    17,    53,    17,
      14,    21,     7,     8,     0,    19,    10,    21,    63,    64,
      15,    66,    63,    64,    12,    91,     3,     4,     5,     6,
      18,    15,    16,    15,    18,    39,    15,    16,    83,    18,
      20,    86,    83,    83,    17,    86,    17,    92,    17,    53,
      11,    92,    92,    14,    58,    15,    16,    20,    18,    63,
      64,    15,    66,    13,    18,    10,    11,    18,    19,    18,
      19,    15,    16,    12,    42,    18,    19,    17,    19,    17,
      84,    17,    86,    17,    19,    18,    17,     7,    15,    86,
      64,    39,    -1,    40
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  BisonParser::yystos_[] =
  {
         0,     9,    23,    24,    17,     0,    10,    25,    30,    15,
      28,    29,    50,    50,    11,    26,    30,    31,    18,    19,
      20,    17,    39,    50,    14,    27,    31,    29,     3,     4,
       5,     6,    49,    40,    41,    50,    17,    17,    18,    19,
      20,    12,    32,    33,    34,    16,    37,    38,    47,    48,
      50,    41,    49,    17,    18,    33,    13,    36,    17,    18,
      38,    47,    48,    17,    21,    42,    35,    48,    50,    17,
      50,     7,     8,    43,    44,    45,    46,    47,    48,    50,
      45,    18,    48,    37,    19,    18,    19,    18,    50,    44,
      18,    17,    37,    18
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  BisonParser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,    40,    41,    44,
      58,    61
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  BisonParser::yyr1_[] =
  {
         0,    22,    23,    24,    25,    25,    25,    26,    26,    26,
      27,    28,    28,    28,    29,    30,    31,    32,    32,    33,
      34,    35,    35,    35,    36,    37,    37,    37,    37,    37,
      37,    37,    38,    39,    40,    40,    40,    41,    42,    43,
      43,    43,    44,    45,    45,    45,    45,    45,    46,    47,
      48,    49,    49,    49,    49,    50
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  BisonParser::yyr2_[] =
  {
         0,     2,     4,     4,     0,     1,     2,     0,     1,     2,
       4,     0,     1,     3,     3,     3,     5,     1,     2,     2,
       4,     0,     1,     2,     4,     0,     1,     1,     1,     2,
       2,     2,     9,     3,     3,     1,     0,     3,     3,     3,
       1,     0,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1
  };


  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const BisonParser::yytname_[] =
  {
    "$end", "error", "$undefined", "FLOAT", "STRING", "OBJECT", "GROUP",
  "FLOATNUMBER", "CHARACTER_STRING", "GLOBALVARS", "DECLARESCRIPT",
  "SCRIPT", "IF", "THEN", "WORLDSCRIPT", "IDENTIFIER", "FOR", "'('", "')'",
  "','", "':'", "'='", "$accept", "eiscript", "globalVars", "declarations",
  "scripts", "worldscript", "globalVarsDefs", "globalVarDef",
  "script_declaration", "script_implementation", "script_body",
  "script_block", "script_if_block", "if_conjunction", "script_then_block",
  "script_then_body", "for_block", "formal_params",
  "formal_parameter_list", "formal_parameter", "params",
  "actual_parameter_list", "actual_parameter", "expression", "variable",
  "assignment", "function_call", "type", "ident", YY_NULL
  };

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const BisonParser::rhs_number_type
  BisonParser::yyrhs_[] =
  {
        23,     0,    -1,    24,    25,    26,    27,    -1,     9,    17,
      28,    18,    -1,    -1,    30,    -1,    25,    30,    -1,    -1,
      31,    -1,    26,    31,    -1,    14,    17,    37,    18,    -1,
      -1,    29,    -1,    28,    19,    29,    -1,    50,    20,    49,
      -1,    10,    50,    39,    -1,    11,    50,    17,    32,    18,
      -1,    33,    -1,    32,    33,    -1,    34,    36,    -1,    12,
      17,    35,    18,    -1,    -1,    48,    -1,    35,    48,    -1,
      13,    17,    37,    18,    -1,    -1,    48,    -1,    47,    -1,
      38,    -1,    37,    48,    -1,    37,    47,    -1,    37,    38,
      -1,    16,    17,    50,    19,    50,    18,    17,    37,    18,
      -1,    17,    40,    18,    -1,    40,    19,    41,    -1,    41,
      -1,    -1,    50,    20,    49,    -1,    17,    43,    18,    -1,
      43,    19,    44,    -1,    44,    -1,    -1,    45,    -1,     7,
      -1,     8,    -1,    46,    -1,    47,    -1,    48,    -1,    50,
      -1,    50,    21,    45,    -1,    50,    42,    -1,     3,    -1,
       4,    -1,     5,    -1,     6,    -1,    15,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  BisonParser::yyprhs_[] =
  {
         0,     0,     3,     8,    13,    14,    16,    19,    20,    22,
      25,    30,    31,    33,    37,    41,    45,    51,    53,    56,
      59,    64,    65,    67,    70,    75,    76,    78,    80,    82,
      85,    88,    91,   101,   105,   109,   111,   112,   116,   120,
     124,   126,   127,   129,   131,   133,   135,   137,   139,   141,
     145,   148,   150,   152,   154,   156
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned char
  BisonParser::yyrline_[] =
  {
         0,    56,    56,    59,    62,    63,    64,    67,    68,    69,
      72,    78,    79,    80,    83,    95,   110,   117,   118,   121,
     124,   127,   128,   129,   132,   135,   136,   137,   138,   139,
     140,   141,   144,   146,   149,   150,   151,   154,   157,   160,
     161,   162,   165,   168,   169,   170,   171,   172,   175,   186,
     189,   192,   193,   194,   195,   198
  };

  // Print the state stack on the debug stream.
  void
  BisonParser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  BisonParser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  BisonParser::token_number_type
  BisonParser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      17,    18,     2,     2,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    20,     2,
       2,    21,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int BisonParser::yyeof_ = 0;
  const int BisonParser::yylast_ = 93;
  const int BisonParser::yynnts_ = 29;
  const int BisonParser::yyempty_ = -2;
  const int BisonParser::yyfinal_ = 5;
  const int BisonParser::yyterror_ = 1;
  const int BisonParser::yyerrcode_ = 256;
  const int BisonParser::yyntokens_ = 22;

  const unsigned int BisonParser::yyuser_token_number_max_ = 271;
  const BisonParser::token_number_type BisonParser::yyundef_token_ = 2;

/* Line 1141 of lalr1.cc  */
#line 4 "eiscript.y"
} // EIScript
/* Line 1141 of lalr1.cc  */
#line 1204 "eiscript.tab.cc"
/* Line 1142 of lalr1.cc  */
#line 201 "eiscript.y"


// We have to implement the error function
void EIScript::BisonParser::error(const EIScript::BisonParser::location_type &loc, const std::string &msg) {
	std::cerr << "Error: " << msg << std::endl;
	std::cerr << "Location: " << loc << std::endl;
}