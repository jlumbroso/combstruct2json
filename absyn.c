#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "absyn.h"

#define ENOUGH 30 // should be enough to hold constructor names and small expressions

extern NodeST* ST; // Symbol Table with all allocated nodes (for cleanup on parse error)

/************************************* Functions *************************************/

ExpressionList* addExpressionToList(Expression* expression, ExpressionList* list)
{
  int size = list->size;
  int space = list->space;
  Expression** components = list->components;

  if (size >= space) { // grow (no need to shrink as list always grows)
    space = 2 * size + 1;
    list->components = (Expression**) realloc(components, space * sizeof(Expression*));
    list->space = space;
  }

  list->components[size] = expression;
  list->size = size + 1;
  return list;
}

StatementList* addStatementToList(Statement* statement, StatementList* list)
{
  int size = list->size;
  int space = list->space;
  Statement** components = list->components;

  if (size >= space) { // grow (no need to shrink as list always grows)    
    space = 2 * size + 1;
    list->components = (Statement**) realloc(components, space * sizeof(Statement*));
    list->space = space;
  }

  list->components[size] = statement;
  list->size = size + 1;
  return list;
}

/*
  Helper function that converts toke types to the corresponding node types.
*/
NodeType tokenToNode(enum yytokentype t)
{
  if (t == EPSILON || t == ATOM || t == Z)
    return UNIT_N;
  else if (t == ID)
    return ID_N;
  else if (t == UNION || t == PROD || t == SUBST)
    return EXPLIST_N;
  else if (t == SET || t == POWERSET || t == SEQUENCE || t == CYCLE)
    return EXP_N;
  else {
    fprintf(stderr, "\nError: token is not a node!\n");
    return UNIT_N;
  }
}

/*
  Free the given node, but not its children. Also takes the corresponding node
  out of the ST.
*/ 
void freeNode(void* node, NodeType type)
{
  key_t key; // will need it to take node out of ST later

  switch (type) {
  case (UNIT_N): ;
    Unit* U = (Unit*) node;
    key = U->key;
    free(U);
    break;
  case (ID_N): ;
    Id* id = (Id*) node;
    key = id->key;
    free(id->name);
    free(id);
    break;
  case (EXP_N): ;
    Expression* E = (Expression*) node;
    key = E->key;
    free(E);
    break;
  case (EXPLIST_N): ;
    ExpressionList* Elist = (ExpressionList*) node;
    key = Elist->key;
    free(Elist->components);
    free(Elist);
    break;
  case (STMT_N): ;
    Statement* S = (Statement*) node;
    key = S->key;
    free(S);
    break;
  case (STMTLIST_N): ;
    StatementList* Slist = (StatementList*) node;
    key = Slist->key;
    free(Slist->components);
    free(Slist);
    break;
  case (ERROR_N): ;
    Error* Err = (Error*) node;
    key = Err->key;
    free(Err->message);
    free(Err);
    break;
  case (GRAMMAR_N): ;
    Grammar* G = (Grammar*) node;
    key = G->key;
    free(G);
    break;
  }

  removeNode(key, ST); // node has been deallocated
}

/*
  Free the abstract syntax subtree with root node. Also takes the corresponding nodes
  out of the ST.
*/ 
void freeNodeRecursive(void* node, NodeType type)
{
  key_t key; // will need it to take node out of ST later

  switch (type) {
  case (UNIT_N): ;
    Unit* U = (Unit*) node;
    key = U->key;
    free(U);
    break;
  case (ID_N): ;
    Id* id = (Id*) node;
    key = id->key;
    free(id->name);
    free(id);
    break;
  case (EXP_N): ;
    Expression* E = (Expression*) node;
    key = E->key;
    freeNodeRecursive(E->component, tokenToNode(E->type));
    free(E);
    break;
  case (EXPLIST_N): ;
    ExpressionList* Elist = (ExpressionList*) node;
    key = Elist->key;
    int size = Elist->size;
    for (int i = 0; i < size; i++) {
      freeNodeRecursive(Elist->components[i], EXP_N);
    }
    free(Elist->components);
    free(Elist);
    break;
  case (STMT_N): ;
    Statement* S = (Statement*) node;
    key = S->key;
    freeNodeRecursive(S->variable, ID_N);
    freeNodeRecursive(S->expression, EXP_N);
    free(S);
    break;
  case (STMTLIST_N): ;
    StatementList* Slist = (StatementList*) node;
    key = Slist->key;
    size = Slist->size;
    for (int i = 0; i < size; i++) {
      freeNodeRecursive(Slist->components[i], STMT_N);
    }
    free(Slist->components);
    free(Slist);
    break;
  case (ERROR_N): ;
    Error* Err = (Error*) node;
    key = Err->key;
    free(Err->message);
    free(Err);
    break;
  case (GRAMMAR_N): ;
    Grammar* G = (Grammar*) node;
    key = G->key;
    if (G->type == ISERROR) {
      freeNodeRecursive(G->component, ERROR_N);
    } else {
      freeNodeRecursive(G->component, STMTLIST_N);
    }
    free(G);
    break;
  }

  removeNode(key, ST); // node has been deallocated
}

/********************************** String Representations **********************************/

/*
  Helper function for converting ints to strings.
*/
char* intToString(long long int a)
{
  int sign = (a >= 0) ? 1 : -1;
  long long int abs = a * sign;
  char* str = (char*) malloc(sizeof(char) * ((int) ceil(log10(abs + 1)) + 2)); // +2: sign and  NULL terminator

  if (sign > 0)
    sprintf(str, "%lld", abs);
  else
    sprintf(str, "-%lld", abs);

  return str;
}

/*
  Helper function for getting string representations of restrictions.
*/
char* restrictionToString(Restriction rest, long long limit)
{
  char* val = intToString(limit);
  char* str = (char*) malloc(sizeof(char) * (strlen(val) + ENOUGH));
  
  switch (rest) {
  case (NONE):
    sprintf(str, "");
    return str;
  case (LESS):
    sprintf(str, ", card <= %s", val);
    return str;
  case (EQUAL):
    sprintf(str, ", card = %s", val);
    return str;
  case (GREATER):
    sprintf(str, ", card >= %s", val);
    return str;
  default:
    sprintf(str, "\nError: restriction is invalid!\n");
    return str;
  }
}

/* 
   String representation of Units. Always encapsulates string literals in malloc'ed char arrays.
*/
char* unitToString(const Unit* U)
{
  char* str = (char*) malloc(sizeof(char) * ENOUGH);

  switch (U->type) {
  case (ATOM): 
    sprintf(str, "Atom");
    return str;
  case (EPSILON):
    sprintf(str, "Epsilon"); 
    return str;
  case (Z): 
    sprintf(str, "Z");
    return str;
  default:
    sprintf(str, "\nError: token is not a unit!\n");
    return str;
  } 
}

/*
  String representation of Ids. We avoid simply returning A->name to enforce the philosophy that
  all strings are owned by this module, so we can (and should) free the returned strings.
*/
char* idToString(const Id* A)
{
  char* str = (char*) malloc(sizeof(char) * (strlen(A->name) + 1)); // NULL terminator
  sprintf(str, "%s", A->name);
  return str;
}

/*
  String representation of expressions.
*/
char* expressionToString(const Expression* E)
{
  char* str = (char*) malloc(sizeof(char) * ENOUGH);

  // type is single entity
  switch (E->type) {
  case (ATOM):
    sprintf(str, "Atom");
    return str;
  case (EPSILON):
    sprintf(str, "Epsilon");
    return str;
  case (Z):
    sprintf(str, "Z");
    return str;
  case (ID):
    free(str);
    Id* id = (Id*) E->component;
    return id->toString(id);
  default: ;
  }

  // type is constructor, but no restrictions apply
  // first line after case must be expression  
  switch (E->type) {
  case (UNION): ;
    ExpressionList* elist = (ExpressionList*) E->component;
    char* subexps = elist->toString(elist);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexps) + ENOUGH));
    sprintf(str, "Union(%s)", subexps);
    free(subexps);
    return str;
  case (PROD): ;
    elist = (ExpressionList*) E->component;
    subexps = elist->toString(elist);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexps) + ENOUGH));
    sprintf(str, "Prod(%s)", subexps);
    free(subexps);
    return str;
  case (SUBST): ;
    elist = (ExpressionList*) E->component;
    subexps = elist->toString(elist);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexps) + ENOUGH));
    sprintf(str, "Subst(%s)", subexps);
    free(subexps);
    return str;
  default: ;
  }
  
  // type is constructor, restrictions apply
  // first line after case must be expression
  switch (E->type) {
  case (SET): ;
    Expression* e = (Expression*) E->component;
    char* rest = restrictionToString(E->restriction, E->limit);
    char* subexp = e->toString(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + ENOUGH));
    sprintf(str, "Set(%s%s)", subexp, rest);
    free(rest);
    free(subexp);
    return str;
  case (POWERSET): ;
    e = (Expression*) E->component;
    rest = restrictionToString(E->restriction, E->limit);
    subexp = e->toString(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + ENOUGH));
    sprintf(str, "PowerSet(%s%s)", subexp, rest);
    free(rest);  
    free(subexp); 
    return str;
  case (SEQUENCE): ;
    e = (Expression*) E->component;
    rest = restrictionToString(E->restriction, E->limit);
    subexp = e->toString(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + ENOUGH));
    sprintf(str, "Sequence(%s%s)", subexp, rest);
    free(rest);
    free(subexp);
    return str;
  case (CYCLE): ;
    e = (Expression*) E->component;
    rest = restrictionToString(E->restriction, E->limit);
    subexp = e->toString(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + ENOUGH));
    sprintf(str, "Cycle(%s%s)", subexp, rest);
    free(rest);
    free(subexp);
    return str;
  default:
    sprintf(str, "\nError: token is not an expression!\n");
    return str;
  }
}

/*
  String representation of lists of expressions.
*/
char* expressionListToString(const ExpressionList* Elist)
{
  int size = Elist->size;
  int length = 0;
  Expression** subexps = Elist->components;
  char** substrs = (char**) malloc(sizeof(char*) * size);
  
  for (int i = 0; i < size; i++) { // get string representation of subexpressions
    char* substr = subexps[i]->toString(subexps[i]);
    substrs[i] = substr;
    length += strlen(substr);
  }

  length += 2 * (size - 1) + 1; // for ", " between subexpressions and NULL terminator
  char* str = (char*) malloc(sizeof(char) * length);
  sprintf(str, "%s", substrs[0]); // there is always a first element

  for (int i = 1; i < size; i++) { // add other elements, separating with comma
    strcat(str, ", ");
    strcat(str, substrs[i]);
    free(substrs[i]);
  }

  free(substrs);
  return str;
}

char* statementToString(const Statement* S)
{
  Id* var = S->variable;
  Expression* exp = S->expression;
  char* varstr = var->toString(var);
  char* expstr = exp->toString(exp);
  char* str = (char*) malloc(sizeof(char) * (strlen(varstr) + strlen(expstr) + 4)); // NULL terminator and " = "
  sprintf(str, "%s = %s", varstr, expstr);
  free(varstr);
  free(expstr);
  return str;
}

char* statementListToString(const StatementList* Slist)
{
  int size = Slist->size;
  int length = 0;
  Statement** substms = Slist->components;
  char** substrs = (char**) malloc(sizeof(char*) * size);
  
  for (int i = 0; i < size; i++) { // get string representation of substatements
    char* substr = substms[i]->toString(substms[i]);
    substrs[i] = substr;
    length += strlen(substr);
  }
  
  length += 2 * (size - 1) + 1; // for ", " between substatements and NULL terminator
  char* str = (char*) malloc(sizeof(char) * length);
  sprintf(str, "%s", substrs[0]); // there is always a first element
  free(substrs[0]);
  
  for (int i = 1; i < size; i++) { // add other elements, separating with comma
    strcat(str, ", ");
    strcat(str, substrs[i]);
    free(substrs[i]);
  }
  
  free(substrs);
  return str;
}

char* errorToString(const Error* error)
{
  char* line = intToString(error->line);
  char* str = (char*) malloc(sizeof(char) * (strlen(error->message) + strlen(line) + ENOUGH));
  if (error->type == LEXER) {
    sprintf(str, "Lexer Error (l. %s): %s", line, error->message);
  } else {
    sprintf(str, "Parser Error (l. %s): %s", line, error->message);
  }

  free(line);
  return str;
}

char* grammarToString(const Grammar* grammar)
{
  if (grammar->type == ISERROR) {
    Error* E = (Error*) grammar->component;
    return E->toString(E);
  } else {
    StatementList* Slist = (StatementList*) grammar->component;
    return Slist->toString(Slist);
  }
}

/********************************** Constructors **********************************/

Unit* newUnit(enum yytokentype type)
{
  Unit* U = malloc(sizeof(Unit));
  U->type = type;
  U->toString = &unitToString;
  U->key = addNode(U, UNIT_N, ST); 
  return U;
}

Id* newId(char* name)
{
  Id* A = malloc(sizeof(Id));
  char* str = (char*) malloc(sizeof(char) * (strlen(name) + 1));
  sprintf(str, "%s", name);
  A->name = str;
  A->toString = &idToString;
  A->key = addNode(A, ID_N, ST); 
  return A;
}

Expression* newExpression(void* component, enum yytokentype type, Restriction restriction, long long int limit)
{
  Expression* E = malloc(sizeof(Expression));
  E->component = component;
  E->type = type;
  E->restriction = restriction;
  E->limit = limit;
  E->toString = &expressionToString;
  E->key = addNode(E, EXP_N, ST);
  return E;
}

ExpressionList* newExpressionList(Expression* expression)
{
  ExpressionList* Elist = malloc(sizeof(ExpressionList));
  Expression** components = (Expression**) malloc(sizeof(Expression*));
  components[0] = expression;
  Elist->components = components;
  Elist->size = 1;
  Elist->space = 1;
  Elist->toString = &expressionListToString;
  Elist->key = addNode(Elist, EXPLIST_N, ST);
  return Elist;
}

Statement* newStatement(Id* variable, Expression* expression)
{
  Statement* S = malloc(sizeof(Statement));
  S->variable = variable;
  S->expression = expression;
  S->toString = &statementToString;
  S->key = addNode(S, STMT_N, ST);
  return S;
}

StatementList* newStatementList(Statement* statement)
{
  StatementList* Slist = malloc(sizeof(StatementList));
  Statement** components = (Statement**) malloc(sizeof(Statement*));
  components[0] = statement;
  Slist->components = components;
  Slist->size = 1;
  Slist->space = 1;
  Slist->toString = &statementListToString;
  Slist->key = addNode(Slist, STMTLIST_N, ST);
  return Slist;
}

Error* newError(int line, char* message, ErrorType type)
{
  Error* E = malloc(sizeof(Error));
  char* str = (char*) malloc(sizeof(char) * (strlen(message) + 1));
  sprintf(str, "%s", message);
  E->message = str;
  E->line = line;
  E->type = type;
  E->toString = &errorToString;
  E->key = addNode(E, ERROR_N, ST);
  return E;
}

Grammar* newGrammar(void* component, GrammarType type)
{
  Grammar* G = malloc(sizeof(Grammar));
  G->component = component;
  G->type = type;
  G->toString = &grammarToString;
  G->key = addNode(G, GRAMMAR_N, ST);
  return G;
}