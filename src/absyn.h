#ifndef ABSYNTYPES
#define ABSYNTYPES
#include "node.h"

/*
  There is a circular dependency between parser.tab.h (which contains the tokens)
  and this file (since parser.tab.h needs the node structures), so we define the
  struct types before including parser.tab.h, and define the actual structs after.
*/

typedef struct Unit_s Unit;
typedef struct Id_s Id;
typedef struct Expression_s Expression;
typedef struct ExpressionList_s ExpressionList;
typedef struct Statement_s Statement;
typedef struct StatementList_s StatementList;
typedef struct Error_s Error;
typedef struct Grammar_s Grammar;

#include "../parser.tab.h"

typedef enum {NONE, LESS, EQUAL, GREATER} Restriction; // restrictions to cardinality

typedef enum {LEXER, PARSER} ErrorType; // origin of error

typedef enum {ISERROR, NOTERROR} GrammarType; // types of grammars resulting from parsing

/***************************** Abstract Syntax Tree Nodes *****************************/

/*
  Node for Atom, Epsilon or Z.
*/
struct Unit_s
{
  enum yytokentype type;
  key_t key;
  char* (*toString)(const struct Unit_s* self);
  char* (*toJson)(const struct Unit_s* self);
};

/*
  Node for Id.
*/
struct Id_s
{
  char* name;
  key_t key;
  char* (*toString)(const struct Id_s* self);
  char* (*toJson)(const struct Id_s* self);
};

/* 
   Node for expression. The component contains the node below the expression in the
   abstract syntax tree, the type indicates what kind of expression this is (Union,
   Prod, Set, Id, Atom, ...), restriction and limit can be used to add restrictions
   to the cardinality.
*/
struct Expression_s
{
  void* component; // could be pointer to Expression, ExpressionList, Id or Unit
  enum yytokentype type;
  Restriction restriction; // restriction type
  long long int limit; // numerical value of restriction in cardinality
  key_t key;
  char* (*toString)(const struct Expression_s* self);
  char* (*toJson)(const struct Expression_s* self);
};

/*
  Node for a list (comma-separated in the input) of expressions.
*/
struct ExpressionList_s
{
  Expression** components;
  int size; // number of expressions in the list of components
  int space; // maximum number of expressions that can be put in the current list
  key_t key;
  char* (*toString)(const struct ExpressionList_s* self);
  char* (*toJson)(const struct ExpressionList_s* self);
};

/*
  Node for a statement. This should be comething of the form a = exp, where
  a is an Id and exp is an expression.
*/
struct Statement_s
{
  Id* variable;
  Expression* expression;
  key_t key;
  char* (*toString)(const struct Statement_s* self);
  char* (*toJson)(const struct Statement_s* self);
};

/*
  Node for a list (comma-separated in the input) of statements.
*/ 
struct StatementList_s
{
  Statement** components;
  int size; // number of statements in the list of components
  int space; // maximum number of statements that can be put in the current list
  key_t key;
  char* (*toString)(const struct StatementList_s* self);
  char* (*toJson)(const struct StatementList_s* self);
};

/*
  Node used to report errors.
*/
struct Error_s
{
  int line;
  char* message;
  ErrorType type;
  key_t key;
  char* (*toString)(const struct Error_s* self);
  char* (*toJson)(const struct Error_s* self);
};

/*
  Root of the abstract syntax tree, can be an error or a list of statements.
*/
struct Grammar_s
{
  GrammarType type;
  void* component; // can be Error or StatementList
  key_t key;
  char* (*toString)(const struct Grammar_s* self);
  char* (*toJson)(const struct Grammar_s* self);
};
#endif

#ifndef ABSYN_H
#define ABSYN_H

/********************************** Constructors **********************************/

Unit* newUnit(enum yytokentype type);

Id* newId(char* name);

Expression* newExpression(void* component, enum yytokentype type, Restriction restriction, long long int limit);

ExpressionList* newExpressionList(Expression* expression);

Statement* newStatement(Id* variable, Expression* expression);

StatementList* newStatementList(Statement* statement);

Error* newError(int line, char* message, ErrorType type);

Grammar* newGrammar(void* component, GrammarType type);

/************************************* Functions *************************************/

ExpressionList* addExpressionToList(Expression* expression, ExpressionList* list);

StatementList* addStatementToList(Statement* statement, StatementList* list);

/*
  Free the abstract syntax tree with root node. Also takes the corresponding nodes out of the ST.
*/
void freeNodeRecursive(void* node, NodeType type);

/*
  Free the given node, but not its children. Also takes the corresponding node out of the ST. 
*/
void freeNode(void* node, NodeType type);

#endif
