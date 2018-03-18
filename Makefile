LEX = flex 
YACC = bison -d # flag is needed to produce parser.tab.h
CC = gcc

exec: parser.tab.c parser.tab.h lex.yy.c absyn.c node.c
	$(CC) -o combstruct2json parser.tab.c lex.yy.c absyn.c node.c

parser.tab.c parser.tab.h: parser.y
	$(YACC) parser.y

lex.yy.c: lexer.l
	$(LEX) lexer.l

absyn.c: absyn.h

absyn.h: parser.tab.h

node.c: node.h

clean:
	rm -f lex.yy.c parser.tab.h parser.tab.c combstruct2json *~ *\#
