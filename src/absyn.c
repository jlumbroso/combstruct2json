#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "absyn.h"

#define ENOUGH 36 // should be enough to hold constructor names and small expressions

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
  Helper function that converts token types to the corresponding node types.
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

/********************************** Json Representations **********************************/

#define MAKE_JSON_ERROR(msg) (strdup("{\n  \"type\": \"error\",\n  \"source\": \"json-export\",\n  \"msg\": \"" msg "\"\n}"))

/*
  Helper function for converting ints to jsons.
*/
// FIXME: fix below
char* intToJson(long long int a)
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
  Helper function for getting json representations of restrictions.
*/
char* restrictionToJson(Restriction rest, long long limit)
{
  // Special case treated separately because it requires no mem allocation.
  char* str = (char*) malloc(sizeof(char));
  switch (rest) {
  case (NONE):
    sprintf(str, "");
    return str;
  default:
      free(str);
  }
  
  char* val = intToJson(limit); 
  str = (char*) malloc(sizeof(char) * (strlen(val) + 2*ENOUGH));
  
  switch (rest) {
  case (LESS):
    sprintf(str, ", \"restriction\": \"card <= %s\"", val);
    return str;
  case (EQUAL):
    sprintf(str, ", \"restriction\": \"card = %s\"", val);
    return str;
  case (GREATER):
    sprintf(str, ", \"restriction\": \"card >= %s\"", val);
    return str;
  default:
    //sprintf(str, "\n{ \"error\": \"restriction is invalid!\" }\n");
    return MAKE_JSON_ERROR("restriction is invalid.");
  }
}

/* 
   Json representation of Units. Always encapsulates json literals in malloc'ed char arrays.
*/ 
// DONE:
char* unitToJson(const Unit* U)
{
  /* NOTE: This is how you would dynamically allocate these strings:

  char* str = (char*) malloc(sizeof(char) * ENOUGH);
  sprintf(str, "{ \"type\": \"unit\", \"unit\": \"Atom\" }");
  return str;

  But since these are literals we can also just return them, they
  are guaranteed to be located in some valid memory address.
  ******************************************************************/

  switch (U->type) {
  case (ATOM):
    return strdup("{ \"type\": \"unit\", \"unit\": \"Atom\" }");
  case (EPSILON):
    return strdup("{ \"type\": \"unit\", \"unit\": \"Epsilon\" }");
  case (Z):
    return strdup("{ \"type\": \"id\", \"id\": \"Z\" }");
  default:
    //return strdup("\n{ \"error\": \"Token is not a unit!\" }\n");
    return MAKE_JSON_ERROR("Token is not a unit.");
  } 
}

/*
  Json representation of Ids. We avoid simply returning A->name to enforce the philosophy that
  all jsons are owned by this module, so we can (and should) free the returned jsons.
*/
// FIXME: fix below
char* idToJson(const Id* A)
{
    char* str = (char*) malloc(sizeof(char) * (strlen(A->name) + 1)); // NULL terminator
  sprintf(str, "%s", A->name);
  return str;
}

/*
  Json representation of expressions.
*/
// DONE:
char* expressionToJson(const Expression* E)
{
  

  // type is single entity
  switch (E->type) {
  case (ATOM):
    //sprintf(str, "{ \"type\": \"unit\", \"unit\": \"Atom\" }");
    return strdup("{ \"type\": \"unit\", \"unit\": \"Atom\" }");
  case (EPSILON):
    //sprintf(str, "{ \"type\": \"unit\", \"unit\": \"Epsilon\" }");
    return strdup("{ \"type\": \"unit\", \"unit\": \"Epsilon\" }");
  case (Z):
    //sprintf(str, "{ \"id\": \"Z\" }"); // Not sure about this one (FIXME: Make it an alias)
    return strdup("{ \"type\": \"id\", \"id\": \"Z\" }");
  case (ID): ;
    //free(str);
    Id* id = (Id*) E->component;
    char *subexp = id->toJson(id);
    char *str = (char*) malloc(sizeof(char) * (strlen(subexp) + 29)); // + 13
    sprintf(str, "{ \"type\": \"id\", \"id\": \"%s\" }", subexp);
    free(subexp);
    return str;
  default: ;
  }

  char* str = (char*) malloc(sizeof(char) * ENOUGH);

  // type is constructor, but no restrictions apply
  // first line after case must be expression  
  switch (E->type) {
  case (UNION): ;
    #define JSONPATT_UNION "{ \"type\": \"op\", \"op\": \"Union\", \"param\": %s }"
    ExpressionList* elist = (ExpressionList*) E->component;
    char* subexps = elist->toJson(elist);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexps) + strlen(JSONPATT_UNION) + 1));
    sprintf(str, JSONPATT_UNION, subexps);
    free(subexps);
    return str;
  case (PROD): ;
    #define JSONPATT_PROD "{ \"type\": \"op\", \"op\": \"Prod\", \"param\": %s }"
    elist = (ExpressionList*) E->component;
    subexps = elist->toJson(elist);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexps) + strlen(JSONPATT_PROD) + 1));
    sprintf(str, JSONPATT_PROD, subexps);
    free(subexps);
    return str;
  case (SUBST): ;
    #define JSONPATT_SUBST "{ \"type\": \"op\", \"op\": \"Subst\", \"param\": %s }"
    elist = (ExpressionList*) E->component;
    subexps = elist->toJson(elist);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexps) + strlen(JSONPATT_SUBST) + 1));
    sprintf(str, JSONPATT_SUBST, subexps);
    free(subexps);
    return str;
  default: ;
  }
  
  // type is constructor, restrictions apply
  // first line after case must be expression
  switch (E->type) {
  case (SET): ;
    #define JSONPATT_SET "{ \"type\": \"op\", \"op\": \"Set\", \"param\": [%s]%s }"
    Expression* e = (Expression*) E->component;
    char* rest = restrictionToJson(E->restriction, E->limit);
    char* subexp = e->toJson(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + strlen(JSONPATT_SET) + 1));
    sprintf(str, JSONPATT_SET, subexp, rest);
    free(rest);
    free(subexp);
    return str;
  case (POWERSET): ;
    #define JSONPATT_POWERSET "{ \"type\": \"op\", \"op\": \"PowerSet\", \"param\": [%s]%s }"
    e = (Expression*) E->component;
    rest = restrictionToJson(E->restriction, E->limit);
    subexp = e->toJson(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + strlen(JSONPATT_POWERSET) + 1));
    sprintf(str, JSONPATT_POWERSET, subexp, rest);
    free(rest);  
    free(subexp); 
    return str;
  case (SEQUENCE): ;
    #define JSONPATT_SEQUENCE "{ \"type\": \"op\", \"op\": \"Sequence\", \"param\": [%s]%s }"
    e = (Expression*) E->component;
    rest = restrictionToJson(E->restriction, E->limit);
    subexp = e->toJson(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + strlen(JSONPATT_SEQUENCE) + 1));
    sprintf(str, JSONPATT_SEQUENCE, subexp, rest);
    free(rest);
    free(subexp);
    return str;
  case (CYCLE): ;
    #define JSONPATT_CYCLE "{ \"type\": \"op\", \"op\": \"Cycle\", \"param\": [%s]%s }"
    e = (Expression*) E->component;
    rest = restrictionToJson(E->restriction, E->limit);
    subexp = e->toJson(e);
    str = (char*) realloc(str, sizeof(char) * (strlen(subexp) + strlen(rest) + strlen(JSONPATT_CYCLE) + 1));
    sprintf(str, JSONPATT_CYCLE, subexp, rest);
    free(rest);
    free(subexp);
    return str;
  default:
    //sprintf(str, "\nError: token is not an expression!\n");
    //return str;
    return MAKE_JSON_ERROR("Token is not an expression.");
  }
}


/*
  Json representation of lists of expressions.
*/
// FIXME: fix below
char* expressionListToJson(const ExpressionList* Elist)
{
  int size = Elist->size;
  int length = 0;
  Expression** subexps = Elist->components;
  char** substrs = (char**) malloc(sizeof(char*) * size);
  
  for (int i = 0; i < size; i++) { // get json representation of subexpressions
    char* substr = subexps[i]->toJson(subexps[i]);
    substrs[i] = substr;
    length += strlen(substr);
  }

  length += 2 * (size - 1) + 1; // for ", " between subexpressions and NULL terminator
  length += 4; // various Json stuff
  char* str = (char*) malloc(sizeof(char) * length);
  sprintf(str, "[ %s", substrs[0]); // there is always a first element

  for (int i = 1; i < size; i++) { // add other elements, separating with comma
    strcat(str, ", ");
    strcat(str, substrs[i]);
    free(substrs[i]);
  }
  strcat(str, " ]");

  free(substrs);
  return str;
}

// DONE:
char* statementToJson(const Statement* S)
{
  Id* var = S->variable;
  Expression* exp = S->expression;
  char* varstr = var->toJson(var);
  char* expstr = exp->toJson(exp);
  char* str = (char*) malloc(sizeof(char) * (strlen(varstr) + strlen(expstr) + 5)); // NULL terminator and "\"': "
  sprintf(str, "\"%s\": %s", varstr, expstr);
  free(varstr);
  free(expstr);
  return str;
}

// DONE:
char* statementListToJson(const StatementList* Slist)
{
  int size = Slist->size;
  int length = 0;
  Statement** substms = Slist->components;
  char** substrs = (char**) malloc(sizeof(char*) * size);
  
  for (int i = 0; i < size; i++) { // get string representation of substatements
    char* substr = substms[i]->toJson(substms[i]);
    substrs[i] = substr;
    length += strlen(substr);
  }
  
  length += 2 * (size - 1) + 1; // for ", " between substatements and NULL terminator
  length += 4; // various Json stuff
  char* str = (char*) malloc(sizeof(char) * length);
  sprintf(str, "{ %s", substrs[0]); // there is always a first element
  free(substrs[0]);
  
  for (int i = 1; i < size; i++) { // add other elements, separating with comma
    strcat(str, ", ");
    strcat(str, substrs[i]);
    free(substrs[i]);
  }
  strcat(str, "}\n");
  
  free(substrs);
  return str;
}

// DONE:
char* errorToJson(const Error* error)
{
  char* line = intToJson(error->line);
  char* str = (char*) malloc(sizeof(char) * (strlen(error->message) + strlen(line) + 2*ENOUGH));
  if (error->type == LEXER) {
      sprintf(str, "{\n  \"type\": \"error\",\n  \"source\": \"lexer\",\n  \"line\": %s\n  \"msg\": \"%s\"\n}", line, error->message);
  } else {
      sprintf(str, "{\n  \"type\": \"error\",\n  \"source\": \"parser\",\n  \"line\": %s\n  \"msg\": \"%s\"\n}", line, error->message);
  }

  free(line);
  return str;
}

// DONE:
char* grammarToJson(const Grammar* grammar)
{
  if (grammar->type == ISERROR) {
    Error* E = (Error*) grammar->component;
    return E->toJson(E);
  } else {
    StatementList* Slist = (StatementList*) grammar->component;
    return Slist->toJson(Slist);
  }
}

/********************************** Constructors **********************************/

Unit* newUnit(enum yytokentype type)
{
  Unit* U = malloc(sizeof(Unit));
  U->type = type;
  U->toString = &unitToString;
  U->toJson = &unitToJson;
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
  A->toJson = &idToJson;
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
  E->toJson = &expressionToJson;
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
  Elist->toJson = &expressionListToJson;
  Elist->key = addNode(Elist, EXPLIST_N, ST);
  return Elist;
}

Statement* newStatement(Id* variable, Expression* expression)
{
  Statement* S = malloc(sizeof(Statement));
  S->variable = variable;
  S->expression = expression;
  S->toString = &statementToString;
  S->toJson = &statementToJson;
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
  Slist->toJson = &statementListToJson;
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
  E->toJson = &errorToJson;
  E->key = addNode(E, ERROR_N, ST);
  return E;
}

Grammar* newGrammar(void* component, GrammarType type)
{
  Grammar* G = malloc(sizeof(Grammar));
  G->component = component;
  G->type = type;
  G->toString = &grammarToString;
  G->toJson = &grammarToJson;
  G->key = addNode(G, GRAMMAR_N, ST);
  return G;
}
