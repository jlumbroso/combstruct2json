# Description of source files

- `lexer.l` contains the token specification used to build a lexer with *flex*.

- `parser.y` contains the grammar rules used to build a parser with *Bison*.

- `absyn.c` and `absyn.h` contain the structures used as nodes in the abstract syntax tree that is constructed during parsing (and stored in a global variable "root"). 

- `node.c` and `node.h` contain code defining a very simple symbol table for nodes of the abstract syntax tree. This is used to free the nodes in case of parsing error. This would not be needed when parsing is successful, but it is necessary when there is an error. The ST is implemented as a linked list, but performance is ok since the only relevant function is `cleanup()`, which runs in linear time. *There could be performance problems when using `freeNodeRecursive()` from `absyn.h`, but this function is currently not being used anywhere.*

- `test1`, `test2`, `test3` and `test4` are very simple test cases for the parser. tests 1 and 3 should parse without errors, test2 should have lexer and parser errors and test4 should have only lexer errors.