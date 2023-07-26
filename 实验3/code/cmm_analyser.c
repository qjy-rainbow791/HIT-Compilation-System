#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "cmm_analyser.h"
#include "cmm.tab.h"

extern Node* syntax_tree;
extern int yyparse();
extern void yyrestart(FILE*);
extern int yylineno;

int gid=1,glabel=1;

FILE* foutput=NULL;

void Error(char type,int line,const char* msg){  //输出错误信息
    //do nothing
}

//变量表
char* varnames[100];    //变量名
int vars[100];          //变量的临时编号（对于数组表示其地址）
int varptr=0;

int find_var(char* str){    //在变量表里查变量编号
    for(int i=0; i < varptr; i++)
        if(strcmp(str,varnames[i]) == 0)
            return vars[i];
    return 0;
}

char* Trans(int var,int k){ //将可能为临时变量或者常量的编号翻译为字符串
    static char str[3][30];
    if(var <= 0)
        sprintf(str[k],"#%d",-var);
    else
        sprintf(str[k],"t%d",var);
    return str[k];
}


int CalcExp(Node* tree,int isleft){
    //对于非条件的Exp生成计算代码并返回临时变量
    //如果是常数x，则返回-x（此时返回值<=0）
    //isleft=1则表示数组，返回的是数组元素地址
    switch(tree->subtype){
    case 0:     //Exp->ID
        return find_var(tree->child->str_val);
    case 1:     //Exp->INT
        return -tree->child->int_val;
    case 3:     //Exp->LP EXP RP
        return CalcExp(tree->child->next,0);
    case 4:     //Exp->ID LP RP
    case 5:{    //Exp->ID LP Args RP
        //Args → Exp COMMA Args | Exp
        char* funcname=tree->child->str_val;
        Node* args=tree->child->next->next;
        static int arglist[30];
        int ptr=0;
        if(!args->isTerminal){
            while(1){
                Node* exp=args->child;
                arglist[ptr++]=CalcExp(exp,0);
                if(args->subtype == 0)  //Args->Exp
                    break;
                args=args->child->next->next;
            }
        }
        //特殊处理输入输出函数
        if(strcmp(funcname,"read") == 0){
            int ret=gid++;
            fprintf(foutput,"READ t%d\n",ret);
            return ret;
        }else if(strcmp(funcname,"write") == 0){
            fprintf(foutput,"WRITE %s\n",Trans(arglist[0],0));
            return 0;   
        }
        for(int i=ptr - 1; i >= 0; i--)
            fprintf(foutput,"ARG %s\n",Trans(arglist[i],0));
        int ret=gid++;
        fprintf(foutput,"t%d := CALL %s\n",ret,funcname);
        return ret;
    }
    case 6:{    //Exp->Exp LB Exp RB
        //保证一维数组
        char* name=tree->child->child->str_val; //第一个Exp一定是Exp->ID
        int baseaddr=find_var(name);    //数组基地址变量编号
        int addr=gid++;
        int index=CalcExp(tree->child->next->next,0);
        fprintf(foutput,"t%d := %s * #4\n",addr,Trans(index,0));
        fprintf(foutput,"t%d := t%d + t%d\n",addr,addr,baseaddr);
        if(isleft)
            return addr;    //数组元素为左值，取地址即可，没必要取值
        int ret=gid++;
        fprintf(foutput,"t%d := *t%d\n",ret,addr);  //否则，用在表达式中，取地址处值
        return ret;
    }
    case 8:{    //Exp->MINUS Exp
        int re=CalcExp(tree->child->next,0);
        int ret=gid++;
        fprintf(foutput,"t%d := #0 - %s\n",ret,Trans(re,0));
        return ret;
    }
    case 10:    //Exp->Exp STAR Exp
    case 11:    //Exp->Exp DIV Exp
    case 12:    //Exp->Exp PLUS Exp
    case 13:{   //Exp->Exp MINUS Exp
        char op=(tree->subtype == 10) ? '*' : (
                (tree->subtype == 11) ? '/' : (
                (tree->subtype == 12) ? '+' : '-'));
        int a=CalcExp(tree->child,0),b=CalcExp(tree->child->next->next,0);
        int ret=gid++;
        fprintf(foutput,"t%d := %s %c %s\n",ret,Trans(a,0),op,Trans(b,1));
        return ret;
    }
    case 17:{   //Exp->Exp ASSIGNOP Exp
        int re=CalcExp(tree->child->next->next,0);
        int le=CalcExp(tree->child,1);
        if(tree->child->subtype == 6)   //左边是数组，返回的是地址，解引用赋值
            fprintf(foutput,"*t%d := %s\n",le,Trans(re,0));
        else
            fprintf(foutput,"t%d := %s\n",le,Trans(re,0));
        return le;  //这个无所谓，反正样例的赋值不会嵌套
    }
    default:{
        printf("Error when calc exp\n");
        exit(-1);
    }
    }
}

void CondExp(Node* condexp,int true_label){
    //处理if/while使用的条件表达式，若真则跳转到指定标签
    //保证条件表达式一定为Exp RELOP Exp或者普通Exp（判断非0）
    if(condexp->subtype == 14){ //Exp->Exp RELOP Exp
        int a=CalcExp(condexp->child,0);
        int b=CalcExp(condexp->child->next->next,0);
        char* rel;
        int relop=condexp->child->next->relop;
        switch(relop){
        case RELOP_EQU: rel="=="; break;
        case RELOP_NEQ: rel="!="; break;
        case RELOP_GE:  rel=">="; break;
        case RELOP_LE:  rel="<="; break;
        case RELOP_GT:  rel=">";  break;
        case RELOP_LT:  rel="<";  break;
        }
        fprintf(foutput,"IF %s %s %s GOTO label%d\n",Trans(a,0),rel,Trans(b,1),true_label);     
    }else{  //普通Exp，先计算然后判断是否非0
        int cond=CalcExp(condexp,0);
        fprintf(foutput,"IF %s != #0 GOTO label%d\n",Trans(cond,0),true_label);
    }
}

//对一般语法树生成中间代码
void Compile(Node* tree){
    //终结符自然结束
    if(tree->isTerminal)
        return;
    switch(tree->type){
    case Unterm_ExtDef:{
        //ExtDef → Specifier FunDec CompSt
        //这里仅处理函数定义，并且specifier一定为INT
        Node* fundec=tree->child->next;
        /*
            VarDec → ID
            FunDec → ID LP VarList RP | ID LP RP
            VarList → ParamDec COMMA VarList | ParamDec
            ParamDec → Specifier VarDec
        */
        Node* varlist=fundec->child->next->next;
        fprintf(foutput,"FUNCTION %s :\n",fundec->child->str_val);
        if(!varlist->isTerminal){   //varlist不为空
            while(1){
                Node* paramdec=varlist->child;
                int arg=gid++;
                fprintf(foutput,"PARAM t%d\n",arg);
                Node* vardec=paramdec->child->next;
                varnames[varptr]=vardec->child->str_val;    
                vars[varptr++]=arg; //连同名字存入变量表
                if(varlist->subtype == 1)   //VarList->ParamDec
                    break;
                varlist=varlist->child->next->next; //VarList → ParamDec COMMA VarList
            }
        }
        Node* compst=tree->child->next->next;
        Compile(compst);
        return;
    }
    case Unterm_CompSt:{
        //CompSt → LC DefList StmtList RC
        /*
        StmtList → Stmt StmtList | e
        Stmt → Exp SEMI | CompSt | RETURN Exp SEMI | IF LP Exp RP Stmt 
            | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
        DefList → Def DefList | e
        Def → Specifier DecList SEMI
        DecList → Dec | Dec COMMA DecList
        Dec → VarDec | VarDec ASSIGNOP Exp
        VarDec → ID | VarDec LB INT RB
        */
        Node* deflist=tree->child->next;
        Node* stmtlist=tree->child->next;
        if(!deflist->isTerminal && deflist->type == Unterm_DefList){
            stmtlist=stmtlist->next;
            //处理deflist中的变量定义
            while(deflist != NULL){     //枚举DefList中的每一行Def
                Node* def=deflist->child;
                Node* declist=def->child->next;
                while(1){               //枚举DecList中的每一个Dec（定义的单个变量）
                    Node* dec=declist->child;
                    Node* vardec=dec->child;
                    int var=gid++;      //新分配一个临时变量
                    if(vardec->subtype == 0){   //VarDec->ID
                        varnames[varptr]=vardec->child->str_val;
                        vars[varptr++]=var;   
                        if(dec->subtype == 1){  //Dec → VarDec ASSIGNOP Exp
                            //处理赋初值表达式
                            int re=CalcExp(dec->child->next->next,0);
                            fprintf(foutput,"t%d := %s\n",var,Trans(re,0));
                        }
                    }
                    if(declist->subtype == 0)   //DecList->Dec
                        break;
                    declist=declist->child->next->next; //DecList → Dec COMMA DecList
                }
                deflist=deflist->child->next;
            }
        }
        if(!stmtlist->isTerminal && stmtlist->type == Unterm_StmtList)
            Compile(stmtlist);      //直接递归处理stmtlist
        return;
    }
    case Unterm_Stmt:{
        /*
        Stmt → Exp SEMI | CompSt | RETURN Exp SEMI | IF LP Exp RP Stmt 
                | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
        */
        switch(tree->subtype){
        case 0: //Stmt->Exp SEMI
            CalcExp(tree->child,0); break;
        case 1: //Stmt->CompSt
            Compile(tree->child); break;
        case 2:{//Stmt->RETURN Exp SEMI
            int ret=CalcExp(tree->child->next,0);
            fprintf(foutput,"RETURN %s\n",Trans(ret,0));
            break;
        }
        case 3: //IF LP Exp RP Stmt
        case 4:{//IF LP Exp RP Stmt ELSE Stmt
            int true_label=glabel++;    //True语句体入口
            int end_label=glabel++;     //if后续语句
            Node* condexp=tree->child->next->next;
            CondExp(condexp,true_label);
            if(tree->subtype == 4)  //IF LP Exp RP Stmt ELSE Stmt
                Compile(tree->child->next->next->next->next->next->next);   
            fprintf(foutput,"GOTO label%d\n",end_label);    
            fprintf(foutput,"LABEL label%d :\n",true_label);
            Compile(tree->child->next->next->next->next);
            fprintf(foutput,"LABEL label%d :\n",end_label);
            break;
        }
        case 5:{    //WHILE LP Exp RP Stmt
            int begin_label=glabel++;   //循环入口
            int true_label=glabel++;    //循环体入口
            int end_label=glabel++;     //循环后续语句
            fprintf(foutput,"LABEL label%d :\n",begin_label);
            Node* condexp=tree->child->next->next;
            CondExp(condexp,true_label);
            fprintf(foutput,"GOTO label%d\n",end_label);    //结束循环
            fprintf(foutput,"LABEL label%d :\n",true_label);
            Compile(tree->child->next->next->next->next);
            fprintf(foutput,"GOTO label%d\n",begin_label);
            fprintf(foutput,"LABEL label%d :\n",end_label);
            break;
        }
        }
        return;
    }
    default:{   //其它非终结符直接无脑递归遍历
        for(Node* ptr=tree->child; ptr != NULL; ptr=ptr->next)
            Compile(ptr);
    }
    }
}

void run(const char* filename){
    syntax_tree=NULL;
    yylineno=1;
    FILE* f=fopen(filename,"r");
    if(f == NULL){
        printf("Error: Cannot open file '%s'\n",filename);
        exit(-1);
    }
    yyrestart(f);
    yyparse();
    fclose(f);
}

int main(int argc,char** argv){
    if(argc < 3){
        printf("Error: Too few args.\n");
        return -1;
    }
    //生成语法树
    run(argv[1]);
    foutput=fopen(argv[2],"w");
    if(foutput == NULL){
        printf("Cannot open output file\n",argv[2]);
        return -1;
    }

    Compile(syntax_tree);

    fclose(foutput);
    return 0;
}
