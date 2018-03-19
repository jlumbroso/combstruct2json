/* 
	Parser for Combinatorial Grammar described in http://www.maplesoft.com/support/help/maple/view.aspx?path=combstruct/specification
*/

%{
#include <stdio.h>
#include <string.h>
#include "src/absyn.h"

Grammar* root; /* root of abstract syntax tree */
NodeST* ST; /* Symbol table with allocated nodes (for cleanup on parse error) */
int hasLexerError = 0;
int yyerror(char *msg);
extern int yylex();
extern FILE* yyin;
extern int lineNumber;
%}

%union 
{
  int symbol;
  Unit* unit;
  Id* id;
  int number;
  Expression* exp;
  ExpressionList* explist;
  Statement* stmt;
  StatementList* stmtlist;
  Grammar* grammar;
}

%token <unit> EPSILON 
%token <unit> ATOM
%token <unit> Z
%token <symbol> UNION
%token <symbol> PROD
%token <symbol> SET
%token <symbol> POWERSET
%token <symbol> SEQUENCE
%token <symbol> CYCLE
%token <symbol> SUBST
%token <symbol> CARD
%token <symbol> LPAR
%token <symbol> RPAR
%token <symbol> COMMA
%token <symbol> LEQ
%token <symbol> GEQ
%token <symbol> EQ
%token <id> ID
%token <number> NUMBER

%type <exp> expression 
%type <explist> expression_list
%type <stmt> statement
%type <stmtlist> statement_list
%type <grammar> grammar

%start grammar

%%

grammar:	   		          statement_list { $$ = newGrammar($1, NOTERROR) ; if (!hasLexerError) root = $$; }
;

statement_list:		                  statement { $$ = newStatementList($1); }
		 			| statement_list COMMA statement { $$ = addStatementToList($3, $1); }
;

statement:			          ID EQ expression { $$ = newStatement($1, $3); }	
;

expression_list: 	                  expression { $$ = newExpressionList($1); }
		 			| expression_list COMMA expression { $$ = addExpressionToList($3, $1); }
;

expression:	   		          EPSILON { $$ = newExpression($1, EPSILON, NONE, 0); }
		 			| ATOM { $$ = newExpression($1, ATOM, NONE, 0); }
		 			| Z { $$ = newExpression($1, Z, NONE, 0); }
		 			| ID { $$ = newExpression($1, ID, NONE, 0); }
		 			| UNION LPAR expression_list RPAR { $$ = newExpression($3, UNION, NONE, 0); }
                                        | PROD LPAR expression_list RPAR { $$ = newExpression($3, PROD, NONE, 0); }
		 			| SUBST LPAR expression_list RPAR { $$ = newExpression($3, SUBST, NONE, 0); }
		 			| SET LPAR expression RPAR { $$ = newExpression($3, SET, NONE, 0); }
                                        | SET LPAR expression COMMA CARD LEQ NUMBER RPAR { $$ = newExpression($3, SET, LESS, $7); }
		          	        | SET LPAR expression COMMA NUMBER GEQ CARD RPAR { $$ = newExpression($3, SET, LESS, $5); }
		 			| SET LPAR expression COMMA CARD EQ NUMBER RPAR { $$ = newExpression($3, SET, EQUAL, $7); }
		 			| SET LPAR expression COMMA NUMBER EQ CARD RPAR { $$ = newExpression($3, SET, EQUAL, $5); }
		 			| SET LPAR expression COMMA CARD GEQ NUMBER RPAR { $$ = newExpression($3, SET, GREATER, $7); }
		 			| SET LPAR expression COMMA NUMBER LEQ CARD RPAR { $$ = newExpression($3, SET, GREATER, $5); }
		 			| POWERSET LPAR expression RPAR { $$ = newExpression($3, POWERSET, NONE, 0); }
		 			| POWERSET LPAR expression COMMA CARD LEQ NUMBER RPAR { $$ = newExpression($3, POWERSET, LESS, $7); }
		 			| POWERSET LPAR expression COMMA NUMBER GEQ CARD RPAR { $$ = newExpression($3, POWERSET, LESS, $5); }
		 			| POWERSET LPAR expression COMMA CARD EQ NUMBER RPAR { $$ = newExpression($3, POWERSET, EQUAL, $7); }
		 			| POWERSET LPAR expression COMMA NUMBER EQ CARD RPAR { $$ = newExpression($3, POWERSET, EQUAL, $5); }
		 			| POWERSET LPAR expression COMMA CARD GEQ NUMBER RPAR { $$ = newExpression($3, POWERSET, GREATER, $7); }
		 			| POWERSET LPAR expression COMMA NUMBER LEQ CARD RPAR { $$ = newExpression($3, POWERSET, GREATER, $5); }
		 			| SEQUENCE LPAR expression RPAR { $$ = newExpression($3, SEQUENCE, NONE, 0); }
                 	                | SEQUENCE LPAR expression COMMA CARD LEQ NUMBER RPAR { $$ = newExpression($3, SEQUENCE, LESS, $7); }
		 			| SEQUENCE LPAR expression COMMA NUMBER GEQ CARD RPAR { $$ = newExpression($3, SEQUENCE, LESS, $5); }
                 	                | SEQUENCE LPAR expression COMMA CARD EQ NUMBER RPAR { $$ = newExpression($3, SEQUENCE, EQUAL, $7); }
		 			| SEQUENCE LPAR expression COMMA NUMBER EQ CARD RPAR { $$ = newExpression($3, SEQUENCE, EQUAL, $5); }
                 	                | SEQUENCE LPAR expression COMMA CARD GEQ NUMBER RPAR { $$ = newExpression($3, SEQUENCE, GREATER, $7); }
		 			| SEQUENCE LPAR expression COMMA NUMBER LEQ CARD RPAR { $$ = newExpression($3, SEQUENCE, GREATER, $5); }
                                        | CYCLE LPAR expression RPAR { $$ = newExpression($3, CYCLE, NONE, 0); }
                 	                | CYCLE LPAR expression COMMA CARD LEQ NUMBER RPAR { $$ = newExpression($3, CYCLE, LESS, $7); }
		 			| CYCLE LPAR expression COMMA NUMBER GEQ CARD RPAR { $$ = newExpression($3, CYCLE, LESS, $5); }
                 	                | CYCLE LPAR expression COMMA CARD EQ NUMBER RPAR { $$ = newExpression($3, CYCLE, EQUAL, $7); }
		 			| CYCLE LPAR expression COMMA NUMBER EQ CARD RPAR { $$ = newExpression($3, CYCLE, EQUAL, $5); }
                 	                | CYCLE LPAR expression COMMA CARD GEQ NUMBER RPAR { $$ = newExpression($3, CYCLE, GREATER, $7); }
		 			| CYCLE LPAR expression COMMA NUMBER LEQ CARD RPAR { $$ = newExpression($3, CYCLE, GREATER, $5); }
;


%%

int reportError(Error* error)
{
  if (error->type == LEXER) {
    hasLexerError = 1;
  }
  root = newGrammar(error, ISERROR);
  char* str = error->toString(error);
  int result = fprintf(stderr, "%s\n", str);
  free(str);
  return result;
}

int yyerror(char *msg)
{
  return reportError(newError(lineNumber, msg, PARSER));
}

Grammar* readGrammar(char* filename)
{
  ST = newNodeST();
  yyin = fopen(filename, "r");
  yyparse();
  fclose(yyin);

  if (root->type == ISERROR) { // we need to clean up
  	// copy error info
  	Error* error = (Error*) root->component;
  	int line = error->line;
  	ErrorType type = error->type;
  	char* str = (char*) malloc(sizeof(char) * (strlen(error->message) + 1));
  	sprintf(str, "%s", error->message);

  	// free all nodes (including root) and their contents, and remove them from ST
  	cleanup(ST);

	// set root to error
  	root = newGrammar(newError(line, str, type), ISERROR);
  }

  // free ST (but not abstract syntax tree nodes) since it is not needed anymore
  free(ST);

  return root;
}

int main(int argc, char* argv[])
{
  readGrammar(argv[1]);
  
  printf("%s\n", root->toJson(root));
}

