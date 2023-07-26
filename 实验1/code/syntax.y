%{
    #include <stdio.h>
    #include <unistd.h>
    #include "syntax_tree.h"
%}

%union {
    Tnode type_tnode;
}

/* terminal */
%token <type_tnode> INT FLOAT
%token <type_tnode> RELOP PLUS MINUS STAR DIV AND OR DOT NOT LP RP LB RB LC RC
%token <type_tnode> SEMI COMMA ASSIGNOP STRUCT RETURN IF ELSE WHILE TYPE ID

%type <type_tnode> Program ExtDef ExtDefList ExtDecList Specifier StructSpecifier OptTag Tag
%type <type_tnode> VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

/* priority */
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left ADD MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB LC RC

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/* high-level definition */
Program:ExtDefList {$$=new_st("Program",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
ExtDefList:ExtDef ExtDefList {$$=new_st("ExtDefList",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | /*emplty*/ {$$=new_st("ExtDefList",0,-1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
ExtDef:Specifier ExtDecList SEMI {$$=new_st("ExtDef",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Specifier SEMI {$$=new_st("ExtDef",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | Specifier FunDec CompSt {$$=new_st("ExtDef",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | error SEMI {$$=new_st("ExtDef",1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    ;
ExtDecList:VarDec {$$=new_st("ExtDefList", 1, $1); nodeList[nodeNum]=$$; nodeNum++;}
    | VarDec COMMA ExtDefList {$$=new_st("ExtDefList",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    ;

/* specifiers */
Specifier:TYPE {$$=new_st("Specifier",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | StructSpecifier {$$=new_st("Specifier",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {$$=new_st("StructSpecifier",5,$1,$2,$3,$4,$5); nodeList[nodeNum]=$$; nodeNum++;}
    | STRUCT Tag {$$=new_st("StructSpecifier",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    ;
OptTag:ID {$$=new_st("OptTag",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | /*empty*/ {$$=new_st("OptTag",0,-1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
Tag:ID {$$=new_st("Tag",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    ;

/* declarators */
VarDec:ID {$$=new_st("VarDec",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | VarDec LB INT RB {$$=new_st("VarDec",4,$1,$2,$3,$4); nodeList[nodeNum]=$$; nodeNum++;}
    ;
FunDec:ID LP VarList RP {$$=new_st("FunDec",4,$1,$2,$3,$4); nodeList[nodeNum]=$$; nodeNum++;}
    | ID LP RP {$$=new_st("FunDec",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    ;
VarList:ParamDec COMMA VarList {$$=new_st("VarList",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | ParamDec {$$=new_st("VarList",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
ParamDec:Specifier VarDec {$$=new_st("ParamDec",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    ;

/* statements */
CompSt:LC DefList StmtList RC {$$=new_st("CompSt",4,$1,$2,$3,$4); nodeList[nodeNum]=$$; nodeNum++;}
    ;
StmtList:Stmt StmtList {$$=new_st("StmtList",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | /*empty*/ {$$=new_st("StmtList",0,-1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
Stmt:Exp SEMI {$$=new_st("Stmt",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | CompSt {$$=new_st("Stmt",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | RETURN Exp SEMI {$$=new_st("Stmt",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{$$=new_st("Stmt",5,$1,$2,$3,$4,$5); nodeList[nodeNum]=$$; nodeNum++;}
    | IF LP Exp RP Stmt ELSE Stmt {$$=new_st("Stmt",7,$1,$2,$3,$4,$5,$6,$7); nodeList[nodeNum]=$$; nodeNum++;}
    | WHILE LP Exp RP Stmt {$$=new_st("Stmt",5,$1,$2,$3,$4,$5); nodeList[nodeNum]=$$; nodeNum++;}
    | error SEMI {$$=new_st("Stmt",1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    ;

/* loacl definitions */
DefList:Def DefList {$$=new_st("DefList",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | /*empty*/ {$$=new_st("DefList",0,-1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
Def:Specifier DecList SEMI {$$=new_st("Def",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | error SEMI {$$=new_st("Def",1,$2); nodeList[nodeNum]=$$; nodeNum++;}
DecList:Dec {$$=new_st("DecList",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | Dec COMMA DecList {$$=new_st("DecList",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
Dec:VarDec {$$=new_st("Dec",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | VarDec ASSIGNOP Exp {$$=new_st("Dec",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}

/* expressions*/
Exp:Exp ASSIGNOP Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp AND Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp OR Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp RELOP Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp PLUS Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp MINUS Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp STAR Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp DIV Exp {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | LP Exp RP {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | MINUS Exp {$$=new_st("Exp",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | NOT Exp {$$=new_st("Exp",2,$1,$2); nodeList[nodeNum]=$$; nodeNum++;}
    | ID LP Args RP {$$=new_st("Exp",4,$1,$2,$3,$4); nodeList[nodeNum]=$$; nodeNum++;}
    | ID LP RP {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp LB Exp RB {$$=new_st("Exp",4,$1,$2,$3,$4); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp DOT ID {$$=new_st("Exp",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | ID {$$=new_st("Exp",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | INT {$$=new_st("Exp",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    | FLOAT {$$=new_st("Exp",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    ;
Args:Exp COMMA Args {$$=new_st("Args",3,$1,$2,$3); nodeList[nodeNum]=$$; nodeNum++;}
    | Exp {$$=new_st("Args",1,$1); nodeList[nodeNum]=$$; nodeNum++;}
    ;

%%

yyerror(char* msg)
{
    printf("Error type B at Line %d: syntax error at \"%s\".\n", yylineno, yytext);
    error_signal = 2;
    error_line[numberOfError] = yylineno;
    stri[numberOfError] = (char *)malloc(sizeof(char)*20);
    strcpy(stri[numberOfError], yytext);
    numberOfError++;
}




