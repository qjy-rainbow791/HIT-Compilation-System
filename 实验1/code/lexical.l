%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "syntax_tree.h"
    #include "syntax.tab.h"
%}

%option yylineno

/* character */
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS \-
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT \!
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}

/* keyword */
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
TYPE "int"|"float"

/* data */
FLOAT ({INT_D}\.[0-9]+)|([0-9]*\.[0-9]*[eE][+-]?[0-9]+)
INT_H 0[xX][0-9a-fA-F]+
INT_O 0[1-7][0-7]*
INT_D 0|[1-9][0-9]*
INT {INT_D}|{INT_O}|{INT_H}

/* comment */
COMMENT ("//".*)|("/*"[^(*/)]*"*/")

/* space */
SPACE [ \f\r\t\v\n]+

/* identifier */
ID [a-zA-Z_][0-9a-zA-Z_]*

%%

{COMMENT} {}
{SPACE} {}

{SEMI} {yylval.type_tnode=new_st("SEMI",0,yylineno);return SEMI;}
{COMMA} {yylval.type_tnode=new_st("COMMA",0,yylineno);return COMMA;}
{ASSIGNOP} {yylval.type_tnode=new_st("ASSIGNOP",0,yylineno);return ASSIGNOP;}
{RELOP} {yylval.type_tnode=new_st("RELOP",0,yylineno);return RELOP;}
{PLUS} {yylval.type_tnode=new_st("PLUS",0,yylineno);return PLUS;}
{MINUS} {yylval.type_tnode=new_st("MINUS",0,yylineno);return MINUS;}
{STAR} {yylval.type_tnode=new_st("STAR",0,yylineno);return STAR;}
{DIV} {yylval.type_tnode=new_st("DIV",0,yylineno);return DIV;}
{AND} {yylval.type_tnode=new_st("AND",0,yylineno);return AND;}
{OR} {yylval.type_tnode=new_st("OR",0,yylineno);return OR;}
{DOT} {yylval.type_tnode=new_st("DOT",0,yylineno);return DOT;}
{NOT} {yylval.type_tnode=new_st("NOT",0,yylineno);return NOT;}
{LP} {yylval.type_tnode=new_st("LP",0,yylineno);return LP;}
{RP} {yylval.type_tnode=new_st("RP",0,yylineno);return RP;}
{LB} {yylval.type_tnode=new_st("LB",0,yylineno);return LB;}
{RB} {yylval.type_tnode=new_st("RB",0,yylineno);return RB;}
{LC} {yylval.type_tnode=new_st("LC",0,yylineno);return LC;}
{RC} {yylval.type_tnode=new_st("RC",0,yylineno);return RC;}

{STRUCT} {yylval.type_tnode=new_st("STRUCT",0,yylineno);return STRUCT;}
{RETURN} {yylval.type_tnode=new_st("RETURN",0,yylineno);return RETURN;}
{IF} {yylval.type_tnode=new_st("IF",0,yylineno);return IF;}
{ELSE} {yylval.type_tnode=new_st("ELSE",0,yylineno);return ELSE;}
{WHILE} {yylval.type_tnode=new_st("WHILE",0,yylineno);return WHILE;}
{TYPE} {yylval.type_tnode=new_st("TYPE",0,yylineno);return TYPE;}

{FLOAT} {yylval.type_tnode=new_st("FLOAT",0,yylineno);return FLOAT;}
{INT} {yylval.type_tnode=new_st("INT",0,yylineno);return INT;}

{ID} {yylval.type_tnode=new_st("ID",0,yylineno);return ID;}

. {error_signal = 1;
    printf("Error type A at line %d: Mysterious characters \'%s\'\n"
    , yylineno, yytext);
    error_line[numberOfError] = yylineno;
    stri[numberOfError] = (char *)malloc(sizeof(char)*40);
    strcpy(stri[numberOfError], yytext);
    numberOfError++;}
%%
