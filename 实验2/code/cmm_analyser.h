/*
    提供了语法树结点的定义
    语法树结点采取左儿子右兄弟的二叉链表表示法
*/

#ifndef CMM_ANALYSER
#define CMM_ANALYSER


typedef struct _node{       //表示一个语法树上的结点（终结符/非终结符）
    int isTerminal;         //终结符
    int type;               //类型
                            //若结点为终结符则取值为cmm.y里定义的token值（例如：INT、FLOAT……）
                            //若结点为非终结符则取值为Unterm_....
    int subtype;            //对于非终结符确定其是哪个产生式
    int line;               //语法树结点的行号（用于报错）
    int mustright;          //表达式是否必须作为右值（解析Exp）
    union{                  //终结符的值
        int int_val;        
        float float_val;    
        char str_val[33];   
    };
    struct _node* next;     //右兄弟指针
    struct _node* child;    //最靠左的儿子指针
}Node;

typedef Node* pNode;

//非终结符类型定义（Node.type的取值）
#define Unterm_Program          0
#define Unterm_ExtDefList       1
#define Unterm_ExtDef           2
#define Unterm_ExtDecList       3
#define Unterm_Specifier        4
#define Unterm_StructSpecifier  5
#define Unterm_OptTag           6
#define Unterm_Tag              7
#define Unterm_VarDec           8
#define Unterm_FunDec           9
#define Unterm_VarList          10
#define Unterm_ParamDec         11
#define Unterm_CompSt           12
#define Unterm_StmtList         13
#define Unterm_Stmt             14
#define Unterm_DefList          15
#define Unterm_Def              16
#define Unterm_DecList          17
#define Unterm_Dec              18
#define Unterm_Exp              19
#define Unterm_Args             20

#endif