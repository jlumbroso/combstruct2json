/* 
	Lexer for Combinatorial Grammar described in http://www.maplesoft.com/support/help/maple/view.aspx?path=combstruct/specification 
*/

%{
#include "src/absyn.h"
#define TOKEN(t) (yylval.symbol = t)
#define UNIT(t) (yylval.unit = newUnit(t))
int lineNumber = 1;
int commentLevel = 0;
extern int reportError(Error* error);
%}

%option noyywrap

%x LINE_COMMENT
%x C_COMMENT

ID	[a-zA-Z][a-zA-Z0-9]*
NUMBER	[0-9]+

%%

"Epsilon"  			{ UNIT(EPSILON); return EPSILON; }
"Atom"				{ UNIT(ATOM); return ATOM; }
"Union"				{ return TOKEN(UNION); }
"Prod"				{ return TOKEN(PROD); }
"Set"				{ return TOKEN(SET); }
"PowerSet"			{ return TOKEN(POWERSET); }
"Sequence"			{ return TOKEN(SEQUENCE); }
"Cycle"				{ return TOKEN(CYCLE); }
"Subst"				{ return TOKEN(SUBST); }
"card"				{ return TOKEN(CARD); }
"Z"			        { UNIT(Z); return Z; }
{ID}				{ yylval.id = newId(yytext); return ID; }
{NUMBER}			{ yylval.number = atoi(yytext); return NUMBER; }
"("			        { return TOKEN(LPAR); }
")"				{ return TOKEN(RPAR); }
","				{ return TOKEN(COMMA); }
"<="|"=<"			{ return TOKEN(LEQ); }
">="|"=>"			{ return TOKEN(GEQ); }
"="				{ return TOKEN(EQ); }
"\n"       			{ lineNumber++; }
" "|"\t"   			{ /* empty */ }
"/*"            	        { commentLevel++; BEGIN(C_COMMENT); }
<C_COMMENT>"/*" 	        { commentLevel++; }
<C_COMMENT>"*/" 	        { commentLevel--; if (commentLevel == 0) BEGIN(INITIAL); }
<C_COMMENT>"\n"   	        { /* empty */ }
<C_COMMENT>.    	        { /* empty */ }
"//"				{ BEGIN(LINE_COMMENT); }
<LINE_COMMENT>"\n"	        { BEGIN(INITIAL); }
<LINE_COMMENT>.		        { /* empty */ }
.          			{ reportError(newError(lineNumber, yytext, LEXER)); }

%%

