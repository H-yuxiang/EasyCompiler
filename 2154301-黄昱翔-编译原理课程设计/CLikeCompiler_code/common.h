#pragma once

// top文件 存放一些全局定义信息

#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <stack>
#include <algorithm>

// 运行控制
#define START_FROM_LEX 1

// 是否命令行打印中间代码
#define IS_SENMANTIC_CMD_SHOW 0

// Lexical
#define KeyWord_NUM 6
#define Delimiter_NUM 7
#define Operator_NUM 38
#define WRONG -1
#define RIGHT 1

#define LC_Wrong_Path "./lexical/lexical-wrong.txt"
#define LC_Rignt_Path "./lexical/lexical-result.txt"
#define PS_Wrong_Path "./parser/parser-wrong.txt"
#define PS_Rignt_Path "./parser/parser-result.txt"

#define PS_Production_Path "./parser/parser-production.txt"
#define PS_ItemGroup_Path "./parser/parser-itemgroup.txt"
#define PS_LR1table_Path "./parser/parser-LR1_table.txt"
#define PS_TREE_Path "./parser/parser-grammar_tree.txt"

#define SM_QUADRUPLE_Path "./parser/parser-4_ruple.txt"

#define PATH_TEST_ARRAY "./test/test-code-array.txt"
#define OC_Path "./objcode/objcode_mips.txt"

// 打开下面注释表示含过程调用，需要修改 common.h 的 Grammar_rule[] 以适应
#define CALL 1

#ifdef CALL
#define Input_Path "./test/test-code-call.txt"
#else
#define Input_Path "./test/test-code-nocall-1.txt"
#endif // CALL


#define ERROR_Pre -10
#define ERROR_KEYWORDS -20
#define ERROR_IDENTIFIER -30
#define ERROR_OPERATOR -40
#define ERROR_DELIMITER -50
#define ERROR_DIGIT -60
#define ERROR_NODFA -70

#define LEXICAL_RIGHT 10

#define ENTRANCE_KEYWORDS 0
#define ENTRANCE_IDENTIFIER 1
#define ENTRANCE_OPERATOR 2
#define ENTRANCE_DELIMITER 3
#define ENTRANCE_DIGIT 4

// Parser
#define ERROR_SHIFT -1
#define ERROR_REDUCE -2
#define ERROR_GOTO -3

#define PARSER_RIGHT 1
#define PARSER_GOING 0

// Semantic
#define QUAD_SATRT 100
#define SEMANTIC_RIGHT 0
#define ERROR_ARGU_NOT_FOUND -1
#define ERROR_ARGU_REPEAT -2
#define ERROR_FUNC_NOT_FOUND -3
#define ERROR_FUNC_REPEAT -4
#define ERROR_MAIN_NOT_FOUND -5
#define ERROR_FUNC_PARA_SIZE -6

// Semantic new
#define ERROR_ARRAY_REPEAT -7
#define ERROR_BAD_ARRAY -8
#define ERROR_ARRAY_NOT_FOUND -9
#define ERROR_ARRAY_DIM -10
#define ERROR_ARRAY_OVERFLOW -11

// Objcode
#define REG_START_IDX 2
#define REG_SIZE 27
#define ERROR_REG_FULL -1
#define OBJGEN_RIGHT 0

//关键字
static std::string KeyWords[KeyWord_NUM] = {
    "int","void","if","else","while","return"
};
//标识符
static std::set<std::string> Identifier;
//数值
static std::set<std::string> Digit;
//界符
static std::string Delimiter[Delimiter_NUM] = {
    ";",
    "(",")",
    "{","}",
    "[","]"
};
//算符
static std::string Operator[Operator_NUM] = {
    "+","++","+=",
    "-","--","-=","->",
    ">",">>",">=",">>=",
    "<","<<","<=","<<=",
    "&","&&","&=",
    "|","||","|=",
    "*","*=","/","/=","%","%=","=","==","!","!=","^","^=",
    ",","?",":",".","~"
};

//文法 - 非终结符定义
#define Grammar_symbol_NUM 34
#ifdef CALL
#define Grammar_rule_NUM 62
#else
#define Grammar_rule_NUM 48
#endif // CALL

#define TERMINAL 0
#define ARGUMENT 1

// 根据需要添加（在原有文法中消除空产生式）
static std::string  Grammar_symbol_name[Grammar_symbol_NUM] = {
    // Grammar_symbol 变元（ID、num做终结符处理 - identifier、digit）
    "Program","类型","ID","形参","语句块",
    "参数列表","参数","内部声明","语句串","内部变量声明",
    "语句","if语句","while语句","return语句","赋值语句",
    "表达式","加法表达式","relop","项","因子",
    "num","FTYPE","call","实参","实参列表",
    "声明串","声明","声明类型","函数声明","变量声明",
    "数组声明","数组","_数组_",
    "epsilon" // 实际消除
};

// 文法 - 非终结符的符号映射（仅换名）
//      - 和上面的 Grammar_symbol_name 对应，可随意修改，数量一致即可
static std::string Grammar_symbol_def[Grammar_symbol_NUM] = {
    "program","type","identifier","formal_para","state_block",
    "para_list","para","inter_declaration","state_string","inter_argument_declaration",
    "state","if_state","while_state","return_state","assign_state",
    "expression","add_expression","relop","item","divisor",
    "number","ftype","call","actual_para","actual_para_list",
    "declaration_string","declaration","declaration type","func_declaration","argu_declaration",
    "array_declaration","array","_array_",
    "epsilon"
};

// 文法 - 语法规则（含过程调用）
//      - 每一个 vector<string> 对象的第一个元素是变元，后续的都是产生式
//      - 消除 epsilon 产生式
static std::vector<std::string> Grammar_rule[Grammar_rule_NUM] = {
    #ifdef CALL
    {"Program","声明串"},
    {"声明串","声明"},
    {"声明串","声明","声明串"},
    {"声明","int","ID","变量声明"},
    {"声明","int","ID","函数声明"},
    {"声明","int","ID","数组声明"},//new1 arr-dec
    {"声明","void","ID","函数声明"},
    {"变量声明",";"},
    {"函数声明","(","形参",")","语句块"},
    {"数组声明","[","num","]",";"},//new2 1d-arr
    {"数组声明","[","num","]","数组声明"},//new3 kd-arr
    #else
    {"Program","类型","ID","(","形参",")","语句块"},
    {"类型","int"},
    {"类型","void"},// todo : more KeyWords
    #endif // CALL
    {"形参","参数列表"},
    {"形参","void"},
    {"参数列表","参数"},
    {"参数列表","参数",",","参数列表"},
    {"参数","int","ID"},
    {"语句块","{","内部声明","语句串","}"},
    {"语句块","{","语句串","}"},
    //{"内部声明","epsilon"},
    {"内部声明","内部变量声明",";"},
    {"内部声明","内部变量声明",";","内部声明"},
    {"内部变量声明","int","ID"},
    {"语句串","语句"},
    {"语句串","语句","语句串"},
    {"语句","if语句"},
    {"语句","while语句"},
    {"语句","return语句"},
    {"语句","赋值语句"},
    {"赋值语句","ID","=","表达式",";"},
    {"赋值语句","数组","=","表达式",";"},//new4 arr=xxx
    {"return语句","return",";"},
    {"return语句","return","表达式",";"},
    {"while语句","while","(","表达式",")","语句块"},
    {"if语句","if","(","表达式",")","语句块"},
    {"if语句","if","(","表达式",")","语句块","else","语句块"},
    {"表达式","加法表达式"},
    {"表达式","(","加法表达式",")"}, // 原本没有
    {"表达式","加法表达式","relop","表达式"},
    {"relop","<"},{"relop","<="},{"relop",">"},{"relop",">="},{"relop","=="},{"relop","!="}, //todo:more Operators
    {"加法表达式","项"},
    {"加法表达式","项","+","加法表达式"},
    {"加法表达式","项","-","加法表达式"},
    {"项","因子"},
    {"项","因子","*","项"},
    {"项","因子","/","项"},
    {"因子","num"},
    {"因子","(","表达式",")"},
    {"因子","ID","call"},
    {"因子","ID"},
    {"因子","数组"},//new5 xxx=arr
    {"数组","ID","[","num","]"},//new6 1d-arr=a[x]
    //{"数组","数组","[","num","]"},//new7 kd-arr=arr[x]
    {"数组","ID","[","num","]","_数组_"},//new8(7.1) 消除左递归
    {"_数组_","[","num","]"},//new9(7.2) 消除左递归
    {"_数组_","[","num","]","_数组_"},//new10(7.3) 消除左递归
    //{"FTYPE","epsilon"},
    //{"FTYPE","call"},
    {"call","(","实参列表",")"},
    {"call","(",")"},
    //{"实参","epsilon"},
    //{"实参","实参列表"},
    {"实参列表","表达式"},
    {"实参列表","表达式",",","实参列表"}
};
