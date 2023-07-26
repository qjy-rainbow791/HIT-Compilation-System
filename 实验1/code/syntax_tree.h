#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

typedef struct tree_node {
    int line;
    char* name;
    struct tree_node *fir_child, *bro;
    union {
        char* id_or_type;
        int it;
        float flt;
    };
} *ST, *Tnode;

extern int yylineno;
extern char* yytext;
extern int error_signal;
extern int error_line[50];
extern char *stri[5];
extern int nodeNum;
extern int numberOfError;
extern Tnode nodeList[10000];
extern int nodeIsChild[10000];
void yyerror(char *msg);
// build new syntax tree
ST new_st(char *name, int num, ...);
// search syntax tree
void search_st(ST st, int level, FILE* fr);
void setChildTag(Tnode node);

int my_atoi(char* text);
int htod(char x);
