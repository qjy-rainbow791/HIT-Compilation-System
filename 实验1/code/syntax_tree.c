#include "syntax_tree.h"

int error_signal = 0;
int error_line[50];
char *stri[];
int numberOfError = 0;

int nodeNum;
Tnode nodeList[10000];
int nodeIsChild[10000];


ST new_st(char *name, int num, ...) {
    Tnode father = (Tnode)malloc(sizeof(struct tree_node));
    Tnode temp = (Tnode)malloc(sizeof(struct tree_node));
    if (!father) {
        yyerror("Failure to Create Treenode");
        exit(0);
    }
    father->name = name;
    va_list list;
    va_start(list, num);

    if (num > 0) {
        temp = va_arg(list, Tnode);
        setChildTag(temp);
        father->fir_child = temp;
        father->line = temp->line;

        if (num > 1) {
            for (int i=1; i<num; i++) {
                temp->bro = va_arg(list, Tnode);
                temp = temp->bro;
                setChildTag(temp);
            }
        }
    }
    else {
        father->line = va_arg(list,int);
        if ((!strcmp(name, "ID")) || (!strcmp(name, "TYPE"))) {
            char *str;
            str = (char *)malloc(sizeof(char)*40);
            strcpy(str, yytext);
            father->id_or_type = str;
        } else if (!strcmp(name, "INT")) {
            father->it = my_atoi(yytext);
        } else if (!strcmp(name, "FLOAT")) {
            father->flt = atof(yytext);
        } else {}
    }
    return father;
}

void search_st(ST st, int level, FILE* fr) {
    if (st != NULL) {
        if (st->line != -1) {
            // space
            for (int i=0; i<level; i++){
                printf("  ");
                fprintf(fr,"\b\b");
            }
            if (st->line != -1) {
                printf("%s", st->name);
                fprintf(fr,"%s", st->name);
                if ((!strcmp(st->name, "ID")) || (!strcmp(st->name, "TYPE"))) {
                    printf(": %s\n", st->id_or_type);
                    fprintf(fr,": %s\n", st->id_or_type);
                } else if (!strcmp(st->name, "INT")) {
                    printf(": %d\n", st->it);
                    fprintf(fr,": %d\n", st->it);
                } else if (!strcmp(st->name, "FLOAT")) {
                    printf(": %f\n", st->flt);
                    fprintf(fr,": %f\n", st->flt);
                } else if ((!strcmp(st->name, "RELOP")) || (!strcmp(st->name, "PLUS")) ||
                        (!strcmp(st->name, "MINUS")) || (!strcmp(st->name, "STAR")) ||
                        (!strcmp(st->name, "DIV")) || (!strcmp(st->name, "AND")) ||
                        (!strcmp(st->name, "OR")) || (!strcmp(st->name, "DOT")) ||
                        (!strcmp(st->name, "NOT")) || (!strcmp(st->name, "RELOP")) ||
                        (!strcmp(st->name, "LP")) || (!strcmp(st->name, "SEMI")) ||
                        (!strcmp(st->name, "RP")) || (!strcmp(st->name, "COMMA")) ||
                        (!strcmp(st->name, "LB")) || (!strcmp(st->name, "ASSIGNOP")) ||
                        (!strcmp(st->name, "RB")) || (!strcmp(st->name, "STRUCT")) ||
                        (!strcmp(st->name, "LC")) || (!strcmp(st->name, "RETURN")) ||
                        (!strcmp(st->name, "RC")) || (!strcmp(st->name, "IF")) ||
                        (!strcmp(st->name, "ELSE")) || (!strcmp(st->name, "WHILE"))) {
                    printf("\n");
                    fprintf(fr,"\n");
                } else {
                    printf("(%d)\n", st->line);
                    fprintf(fr,"(%d)\n", st->line);
                }
            }
            search_st(st->fir_child, level+1, fr);
            search_st(st->bro, level, fr);
        } else {
            search_st(st->fir_child, level+1, fr);
            search_st(st->bro, level, fr);
        }
    }
}

void setChildTag(Tnode node) {
    for (int i=0; i<nodeNum; i++) {
        if (nodeList[i] == node)
            nodeIsChild[i] = 1;
    }
}

int my_atoi(char* text) {
    if (text[0] == '0' && text[1] == 'x' || text[0] == '0' && text[1] == 'X' ) {
        int sum = 0;
        int len = strlen(text);
        for (int i=0; i<len-2; i++)
            sum += (int)(pow(16,(double)i)) * htod(text[len-1-i]);
        return sum;
    } else if (text[0] == '0' && text[1] != 'x' && text[1] != 'X' ) {
        int sum = 0;
        int len = strlen(text);
        for (int i=0; i<len; i++)
            sum += (int)(pow(8,(double)i)) * (int)(text[len-1-i]-'0');
        return sum;
    } else {
        return atoi(text);
    }
}

int htod(char x) {
    if (x>='0' && x<='9') {
        return (int)(x - '0');
    } else {
        switch(x) {
            case 'A': return 10;
            case 'a': return 10;
            case 'B': return 11;
            case 'b': return 11;
            case 'C': return 12;
            case 'c': return 12;
            case 'D': return 13;
            case 'd': return 13;
            case 'E': return 14;
            case 'e': return 14;
            case 'F': return 15;
            case 'f': return 15;
        }

    }
}

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* fr = fopen("result.txt", "wr+");
    for (int i=1; i<argc; i++) {
        nodeNum = 0;
        memset(nodeList, 0, sizeof(Tnode)*10000);
        memset(nodeIsChild, 0, sizeof(int)*10000);
        error_signal = 0;
        for(int i = 0;i < 50;i++)
            error_line[i] = 0;
        yylineno = 1;
        numberOfError = 0;

        FILE* f = fopen(argv[i], "r");
        if (!f) {
            perror(argv[i]);
            return 1;
        }
        fprintf(fr,"/********** %s **********/\n", argv[i]);
        printf("/********** %s **********/\n", argv[i]);
        yyrestart(f);
        yyparse();
        fclose(f);
        if (error_signal == 1) {
            for(int i = 0;i < numberOfError; i++)
            fprintf(fr, "Error type A at line %d: Mysterious characters \'%s\'\n", error_line[i], stri[i]);
            continue;
        }
        else if(error_signal == 2){
            for(int i = 0;i < numberOfError; i++)
            fprintf(fr, "Error type B at Line %d: syntax error at \"%s\".\n", error_line[i], stri[i]);
            continue;
        }
        for (int j=0; j<nodeNum; j++) {
            if (nodeIsChild[j] != 1)
                search_st(nodeList[j], 0, fr);
        }
    }
    fclose(fr);
    return 0;
}
