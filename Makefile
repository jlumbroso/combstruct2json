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

# FIXME: Is this the best way of generating a self-contained header for the library?
combstruct2json.h: src/parser.y parser.tab.h src/absyn.h src/node.h
	awk '/#ifndef YYTOKENTYPE/{flag=1} flag {print} /#endif/{flag=0}' parser.tab.h > c2jh_yytokentype
	awk '/#ifndef NODESTTYPE/{flag=1} flag {print} /#endif/{flag=0}' src/node.h > c2jh_nodesttype
	awk '/#ifndef ABSYNTYPES/{flag=1} flag {print} /#endif/{flag=0}' src/absyn.h > c2jh_core
	sed -i -e '/#include "..\/parser.tab.h"/{r c2jh_yytokentype' -e 'd}' c2jh_core
	sed -i -e '/#include "node.h"/{r c2jh_nodesttype' -e 'd}' c2jh_core
	mv c2jh_core combstruct2json.h

	echo "#ifndef C2J_H" >> combstruct2json.h
	echo "#define C2H_H" >> combstruct2json.h
	echo "Grammar* readGrammar(char* filename);" >> combstruct2json.h
	echo "#endif" >> combstruct2json.h

	rm -f c2jh_yytokentype c2jh_nodesttype


combstruct2json.o: combstruct2json.h libcombstruct2json.a pyext/wrapper.c setup.py setup.cfg
	python setup.py build_ext --inplace


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

lib: libcombstruct2json.a combstruct2json.h combstruct2json.o

all: exec lib


clean:
	rm -f lex.yy.c parser.tab.h parser.tab.c 
	rm -f parser.tab.o lex.yy.o absyn.o node.o
	rm -f *~ *\# src/*~ src/*\# tests/*~ tests/*\#
	rm -f c2jh_yytokentype c2jh_nodesttype c2jh_core
	rm -f combstruct2json.h
	rm -f combstruct2json libcombstruct2json.a
	rm -Rf build combstruct2json.so
