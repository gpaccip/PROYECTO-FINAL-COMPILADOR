%{
#include "ast.h"
#include "parser.hpp"
#include <string>

int current_line = 1;
int current_col = 1;

void add_tok(const std::string& name) {
    lexical_tokens.push_back({name, yytext, current_line, current_col});
    current_col += yyleng;
}
%}
%option noyywrap
%%
"entero"                { add_tok("ENTERO"); yylval.str = new std::string(yytext); return ENTERO_T; }
"funcion_matematica"    { add_tok("FUNCION_MAT"); yylval.str = new std::string(yytext); return FUNCION_MAT_T; }
"derivar"               { add_tok("DERIVAR"); return DERIVAR; }
"integrar_indefinida"   { add_tok("INTEGRAR"); return INTEGRAR_INDEFINIDA; }
"leer"                  { add_tok("LEER"); return LEER; }
"imprimir"              { add_tok("ESCRIBIR"); return IMPRIMIR; }

[a-zA-Z_][a-zA-Z0-9_]*  { add_tok("IDENTIFICADOR"); yylval.str = new std::string(yytext); return ID; }
[0-9]+                  { add_tok("NUM_ENTERO"); yylval.str = new std::string(yytext); return NUM_ENTERO; }
"\""[^\"]*"\""          { add_tok("LITERAL_CADENA"); 
                          std::string txt = yytext; 
                          yylval.str = new std::string(txt.substr(1, txt.length()-2)); 
                          return CADENA; }

"+"                     { add_tok("MAS"); return '+'; }
"*"                     { add_tok("POR"); return '*'; }
"^"                     { add_tok("POTENCIA"); return '^'; }
"="                     { add_tok("ASIGNAR"); return '='; }
";"                     { add_tok("PUNTOCOMA"); return ';'; }
","                     { add_tok("COMA"); return ','; }
"("                     { add_tok("PARENIZQ"); return '('; }
")"                     { add_tok("PARENDER"); return ')'; }

[ \t]+                  { current_col += yyleng; }
\r                      { }
\n                      { current_line++; current_col = 1; }
.                       { add_tok("ERROR"); total_errors++; }
%%
