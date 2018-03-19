LEX = flex 
YACC = bison -d # flag is needed to produce parser.tab.h
CC = gcc

combstruct2json: parser.tab.c parser.tab.h lex.yy.c src/absyn.c src/node.c
	$(CC) -o combstruct2json parser.tab.c lex.yy.c src/absyn.c src/node.c

parser.tab.c src/parser.tab.h: src/parser.y
	$(YACC) src/parser.y

lex.yy.c: src/lexer.l
	$(LEX) src/lexer.l

src/absyn.c: src/absyn.h

src/absyn.h: parser.tab.h

src/node.c: src/node.h

clean:
	rm -f lex.yy.c parser.tab.h parser.tab.c combstruct2json *~ *\# src/*~ src/*\# tests/*~ tests/*\#
