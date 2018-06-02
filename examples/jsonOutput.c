#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "../combstruct2json.h"



/*****************************************************************
 * FULL EXAMPLE OF COMBSTRUCT ABSTRACT TREE TRAVERSAL
 * 
 * This example illustrates the full code needed to convert a
 * parsed "combstruct" abstract tree and convert it into a JSON
 * string. This should provide guidance on how to manipulate a
 * parsed tree.
 * 
 * Assuming the library and header have been built, and the
 * working directory is the top-level directory of this project:
 * 
 * $ gcc -o jsonOutput -L. -lcombstruct2json examples/jsonOutput.c
 * $ ./jsonOutput tests/cographs
 * 
 *****************************************************************/

char* treewalkGrammar(const Grammar* grammar);

int main(int argc, char* argv[])
{
  Grammar *root = readGrammar(argv[1]);
  printf("%s\n", treewalkGrammar(root));

  /* Naturally, one could also use the built-in toJson converter:

  printf("%s\n", root->toJson(root));

  */

  return 0;
}



/*****************************************************************
 * FULL EXAMPLE OF COMBSTRUCT ABSTRACT TREE TRAVERSAL
 * 
 * This example illustrates the full code needed to convert a
 * parsed "combstruct" abstract tree and convert it into a JSON
 * string. This should provide guidance on how to manipulate a
 * parsed tree.
 *****************************************************************/

#define ENOUGH 36
#define MAKE_JSON_ERROR(msg) (strdup("{\n  \"type\": \"error\",\n  \"source\": \"json-export\",\n  \"msg\": \"" msg "\"\n}"))

char* treewalkInt(long long int a)
{
  int sign = (a >= 0) ? 1 : -1;
  long long int abs = a * sign;

  // String mem alloc ============================================
  char* str = (char*) malloc(sizeof(char) * (
    (int) ceil(log10(abs + 1)) +   // Length of the base-10 number
    2));                           // sign + NULL terminator
  // =============================================================

  if (sign > 0) sprintf(str, "%lld", abs);
  else sprintf(str, "-%lld", abs);

  return str;
}

char* treewalkRestriction(Restriction rest, long long limit)
{
  // Special case treated separately because it requires no
  // memory allocation.

  switch (rest)
  {
  case (NONE):
    return strdup("");

  default: ;
  }
  
  char* val = treewalkInt(limit);

  // String mem alloc ============================================
  char* str = (char*) malloc(sizeof(char) * (
    strlen(val) +                  // length of base-10 number
    2*ENOUGH));                    // padding for the extra text
  // =============================================================
  
  switch (rest)
  {
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
    return MAKE_JSON_ERROR("restriction is invalid.");
  }
}

char* treewalkUnit(const Unit* U)
{
  // No memory allocation needed here, as we just duplicate
  // constant strings.

  switch (U->type)
  {
  case (ATOM):
    return strdup("{ \"type\": \"unit\", \"unit\": \"Atom\" }");

  case (EPSILON):
    return strdup("{ \"type\": \"unit\", \"unit\": \"Epsilon\" }");

  case (Z):
    return strdup("{ \"type\": \"id\", \"id\": \"Z\" }");

  default:
    return MAKE_JSON_ERROR("Token is not a unit.");
  }
}

char* treewalkId(const Id* A)
{
  // String mem alloc ============================================
  char* str = (char*) malloc(sizeof(char) * (
    strlen(A->name) +              // length of the string name
    1));                           // NULL terminator
  // =============================================================

  sprintf(str, "%s", A->name);

  return str;
}

char* treewalkExpressionList(const ExpressionList* Elist);

char* treewalkExpression(const Expression* E)
{
  // CASE 1: type is single entity
  switch (E->type)
  {
  case (ATOM):
    return strdup("{ \"type\": \"unit\", \"unit\": \"Atom\" }");

  case (EPSILON):
    return strdup("{ \"type\": \"unit\", \"unit\": \"Epsilon\" }");

  case (Z):
    return strdup("{ \"type\": \"id\", \"id\": \"Z\" }");

  case (ID): ;
    #define JSONPATT_ID "{ \"type\": \"id\", \"id\": \"%s\" }"

    Id* id = (Id*) E->component;
    char *subexp = treewalkId(id);
    const char *fmt_pattern = JSONPATT_ID;

    // String mem alloc ============================================
    char *str = (char*) malloc(sizeof(char) * (
      strlen(subexp) +               // length of the string name
      strlen(fmt_pattern) +          // pattern length
      1));                           // NULL terminator
    // =============================================================
    
    sprintf(str, fmt_pattern, subexp);
    free(subexp);
    return str;

  default: ;
  }

  char* str = (char*) malloc(sizeof(char) * ENOUGH);

  // CASE 2: type is constructor, but no restrictions apply
  if (E->type == UNION || E->type == PROD || E->type == SUBST)
  {
    const char *fmt_pattern;

    #define JSONPATT_UNION "{ \"type\": \"op\", \"op\": \"Union\", \"param\": %s }"
    #define JSONPATT_PROD "{ \"type\": \"op\", \"op\": \"Prod\", \"param\": %s }"
    #define JSONPATT_SUBST "{ \"type\": \"op\", \"op\": \"Subst\", \"param\": %s }"

    switch (E->type)
    {
    case (UNION): fmt_pattern = JSONPATT_UNION;
    case (PROD):  fmt_pattern = JSONPATT_PROD;
    case (SUBST): fmt_pattern = JSONPATT_SUBST;
    default: ;
    }

    ExpressionList* elist = (ExpressionList*) E->component;
    char* subexps = treewalkExpressionList(elist);
    
    // String mem alloc ============================================
    str = (char*) realloc(str, sizeof(char) * (
      strlen(subexps) +              // length of the subexpressions
      strlen(fmt_pattern) +          // pattern length
      1));                           // NULL terminator
    // =============================================================

    sprintf(str, fmt_pattern, subexps);
    free(subexps);

    return str;
  }
  
  // CASE 3: type is constructor, and can have restrictions
  if (E->type == SET || E->type == POWERSET || E->type == SEQUENCE || E->type == CYCLE)
  {
    const char *fmt_pattern;

    #define JSONPATT_SET "{ \"type\": \"op\", \"op\": \"Set\", \"param\": [%s]%s }"
    #define JSONPATT_POWERSET "{ \"type\": \"op\", \"op\": \"PowerSet\", \"param\": [%s]%s }"
    #define JSONPATT_SEQUENCE "{ \"type\": \"op\", \"op\": \"Sequence\", \"param\": [%s]%s }"
    #define JSONPATT_CYCLE "{ \"type\": \"op\", \"op\": \"Cycle\", \"param\": [%s]%s }"

    switch (E->type)
    {
    case (SET):      fmt_pattern = JSONPATT_UNION;
    case (POWERSET): fmt_pattern = JSONPATT_PROD;
    case (SEQUENCE): fmt_pattern = JSONPATT_SUBST;
    case (CYCLE):    fmt_pattern = JSONPATT_CYCLE;

    default: ;
    }

    Expression* e = (Expression*) E->component;
    char *rest = treewalkRestriction(E->restriction, E->limit);
    char *subexp = treewalkExpression(e);
    
    // String mem alloc ============================================
    str = (char*) realloc(str, sizeof(char) * (
      strlen(subexp) +              // length of the subexpression
      strlen(rest) +                // length of the restriction
      strlen(fmt_pattern) +         // pattern length
      1));                          // NULL terminator
    // =============================================================

    sprintf(str, fmt_pattern, subexp, rest);
    free(rest);
    free(subexp);

    return str;
  }

  return MAKE_JSON_ERROR("Token is not an expression.");
}

char* treewalkStatement(const Statement* S)
{
  Id* var = S->variable;
  Expression* exp = S->expression;
  char* varstr = treewalkId(var);
  char* expstr = treewalkExpression(exp);

  #define JSONPATT_STATEMENT "\"%s\": %s"

  // String mem alloc ============================================
  char* str = (char*) malloc(sizeof(char) * (
    strlen(varstr) +              // length of the symbol
    strlen(expstr) +              // length of the statement
    strlen(JSONPATT_STATEMENT) +  // length of pattern (4)
    1));                          // NULL terminator
  // =============================================================

  sprintf(str, JSONPATT_STATEMENT, varstr, expstr);
  free(varstr);
  free(expstr);
  return str;
}

typedef char* (recStringFunc)(const void* param);

char* treewalkAnyList(void** subexps, int size, recStringFunc recfunc, const char *openChar, const char *closeChar)
{
  char** substrs = (char**) malloc(sizeof(char*) * size);

  int totalCharLength = 0;
  
  for (int i = 0; i < size; i++) {
    char* substr = recfunc(subexps[i]);
    substrs[i] = substr;

    // Add characters taken up by this subexpression to total.
    totalCharLength += strlen(substr);
  }

  // String mem alloc ============================================
  char *str = (char*) malloc(sizeof(char) * (
    totalCharLength +       // total length of all subexpressions
    2 * (size - 1) +        // separators ", " between elements
    strlen(openChar) +      // characters to open the list
    strlen(closeChar) +     // characters to close the list
    2 +                     // additional ("  ") characters
    1));                    // NULL terminator
  // =============================================================

  // There is always a first element ...
  sprintf(str, "%s %s", openChar, substrs[0]);

  // ... add other elements, separating with a commma ...
  for (int i = 1; i < size; i++) {
    strcat(str, ", ");
    strcat(str, substrs[i]);
    free(substrs[i]);
  }
  
  // ... close the list.
  strcat(str, " ");
  strcat(str, closeChar);

  free(substrs);
  return str;
}

char* treewalkExpressionList(const ExpressionList* Elist)
{
  return treewalkAnyList(
    (void **)Elist->components,
    Elist->size,
    treewalkExpression,
    "[",
    "]");
}

char* treewalkStatementList(const StatementList* Slist)
{
  return treewalkAnyList(
    (void **)Slist->components,
    Slist->size,
    treewalkStatement,
    "{",
    "}\n");
}

#define JSONPATT_ERROR_LEXER   "{\n  \"type\": \"error\",\n  \"source\": \"lexer\",\n  \"line\": %s\n  \"msg\": \"%s\"\n}"
#define JSONPATT_ERROR_PARSER  "{\n  \"type\": \"error\",\n  \"source\": \"parser\",\n  \"line\": %s\n  \"msg\": \"%s\"\n}"

char* treewalkError(const Error* error)
{
  char* line = treewalkInt(error->line);

  // String mem alloc ============================================
  char* str = (char*) malloc(sizeof(char) * (
    strlen(error->message) +        // length of the error message
    strlen(line) +                  // length of the line number
    strlen(JSONPATT_ERROR_PARSER) + // length of the pattern
    ENOUGH));                       // padding
  // =============================================================

  switch(error->type)
  {
    case LEXER:  sprintf(str, JSONPATT_ERROR_LEXER, line, error->message);
    case PARSER: sprintf(str, JSONPATT_ERROR_PARSER, line, error->message);
    default: ;
  }

  free(line);
  return str;
}

char* treewalkGrammar(const Grammar* grammar)
{
  if (grammar->type == ISERROR) {
    Error* E = (Error*) grammar->component;
    return treewalkError(E);
  } else {
    StatementList* Slist = (StatementList*) grammar->component;
    return treewalkStatementList(Slist);
  }
}