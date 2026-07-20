%{
#include "ast.h"
#include <iostream>
#include <string>
extern int yylex();
void yyerror(const char* s);
BlockNode* rootAST = nullptr;
int total_errors = 0;
%}
%union {
    std::string* str;
    ASTNode* node;
    BlockNode* block;
    DataType type;
}
%token <str> ID NUM_ENTERO CADENA ENTERO_T FUNCION_MAT_T
%token LEER IMPRIMIR DERIVAR INTEGRAR_INDEFINIDA
%type <block> programa lista_sentencias
%type <node> sentencia declaracion asignacion io_stmt expresion
%left '+' '-'
%left '*' '/'
%right '^'
%%
programa: lista_sentencias { rootAST = $1; };
lista_sentencias: lista_sentencias sentencia { $1->add($2); $$ = $1; } | { $$ = new BlockNode(); };
sentencia: declaracion ';' { $$ = $1; } | asignacion ';' { $$ = $1; } | io_stmt ';' { $$ = $1; };
declaracion
    : ENTERO_T ID { $$ = new VarDeclNode(DataType::ENTERO, *$2); }
    | FUNCION_MAT_T ID { $$ = new VarDeclNode(DataType::FUNCION_MATEMATICA, *$2); }
    ;
asignacion: ID '=' expresion { $$ = new AssignmentNode(*$1, $3); };
io_stmt
    : LEER '(' ID ')' { $$ = new IONode("leer", *$3); }
    | IMPRIMIR '(' CADENA ',' ID ')' { $$ = new IONode("imprimir", *$5, *$3); }
    ;
expresion
    : expresion '+' expresion { $$ = new BinaryOpNode("+", $1, $3); }
    | expresion '*' expresion { $$ = new BinaryOpNode("*", $1, $3); }
    | expresion '^' expresion { $$ = new BinaryOpNode("^", $1, $3); }
    | DERIVAR '(' ID ',' ID ')' { $$ = new MathNativeNode("derivar", *$3, *$5); }
    | INTEGRAR_INDEFINIDA '(' ID ',' ID ')' { $$ = new MathNativeNode("integrar_indefinida", *$3, *$5); }
    | NUM_ENTERO { $$ = new LiteralNode(*$1, DataType::ENTERO); }
    | ID { $$ = new VariableNode(*$1); }
    ;
%%
void yyerror(const char* s) { total_errors++; }
