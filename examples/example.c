#include <stdlib.h>
#include <stdio.h>

#include "../combstruct2json.h"



/*****************************************************************
 * MINIMAL EXAMPLE OF COMBSTRUCT2JSON CALL
 * 
 * This example shows how to link to the combstruct2json library
 * and use built-in functionality to parse a combstruct
 * specification, and then use the built-in string conversion
 * to output it.
 * 
 * Assuming the library and header have been built, and the
 * working directory is the top-level directory of this project:
 * 
 * $ gcc -o example -L. -lcombstruct2json examples/example.c
 * $ ./example tests/cographs
 * 
 *****************************************************************/

int main(int argc, char* argv[])
{
  Grammar *root = readGrammar(argv[1]);
  printf("String format: %s\n\n", root->toString(root));
  printf("JSON format: %s\n\n", root->toJson(root));
  return 0;
}