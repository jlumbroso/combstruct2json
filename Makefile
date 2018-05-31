LEX = flex 
YACC = bison -d # flag is needed to produce parser.tab.h
CC = gcc
AR = ar rc
RANLIB = ranlib


combstruct2json: parser.tab.c parser.tab.h lex.yy.c src/absyn.c src/node.c
	$(CC) -o combstruct2json parser.tab.c lex.yy.c src/absyn.c src/node.c

libcombstruct2json.a: parser.tab.o lex.yy.o absyn.o node.o
	$(AR) libcombstruct2json.a parser.tab.o lex.yy.o absyn.o node.o
	$(RANLIB) libcombstruct2json.a


parser.tab.c src/parser.tab.h: src/parser.y
	$(YACC) src/parser.y

lex.yy.c: src/lexer.l
	$(LEX) src/lexer.l

src/absyn.c: src/absyn.h

src/absyn.h: parser.tab.h

src/node.c: src/node.h


parser.tab.o: parser.tab.c parser.tab.h
	$(CC) -D _COMPILE_LIB -c parser.tab.c

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

absyn.o: src/absyn.c
	$(CC) -c src/absyn.c

node.o: src/node.c
	$(CC) -c src/node.c


exec: combstruct2json

lib: libcombstruct2json.a

all: exec lib


clean:
	rm -f lex.yy.c parser.tab.h parser.tab.c 
	rm -f parser.tab.o lex.yy.o absyn.o node.o
	rm -f *~ *\# src/*~ src/*\# tests/*~ tests/*\#
	rm -f combstruct2json libcombstruct2json.a
