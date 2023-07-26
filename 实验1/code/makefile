cfile = syntax.tab.c syntax_tree.c lex.yy.c
target = parser
bison_file = syntax.y lex.yy.c syntax_tree.c syntax_tree.h
bison_target = syntax.tab.c syntax.tab.h
flex_file = lexical.l syntax_tree.c syntax_tree.h

$(target):$(cfile)
	gcc $^ -lfl -ly -lm -o $@
$(bison_target):$(bison_file)
	bison -d syntax.y
lex.yy.c:$(flex_file)
	flex lexical.l