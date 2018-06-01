#include <stdlib.h>
#include <stdio.h>
#include "combstruct2json.h"

/* Minimal example of how to use this library. */
// make all && gcc -o foo -L. -lcombstruct2json example.c && ./foo tests/cographs

int main(int argc, char* argv[])
{
  Grammar *root = readGrammar(argv[1]);
  
  printf("%s\n", root->toJson(root));
  return 1;
}