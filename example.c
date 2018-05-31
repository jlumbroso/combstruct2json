#include <stdlib.h>
#include <stdio.h>
#include "src/absyn.h"

Grammar* readGrammar(char* filename);

int main(int argc, char* argv[])
{
  Grammar *root = readGrammar(argv[1]);
  
  printf("%s\n", root->toJson(root));
  return 1;
}