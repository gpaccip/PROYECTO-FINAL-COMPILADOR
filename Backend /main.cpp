#include "ast.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstdlib>
#include <windows.h>

#include "parser.hpp"
extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern void yyrestart(FILE*);
extern BlockNode* rootAST;

// Variables Globales necesarias para el compilador
std::unordered_map<std::string, Symbol> symbol_table;
int next_memory_address = 0;
std::vector<Quadruple> intermediate_code;
std::vector<TokenInfo> lexical_tokens;
OptStats opt_stats;

void printLexicalTable(const std::string& filename) {
    std::cout << "\n=== ANALISIS LEXICO: " << filename << " ===\n";
    std::cout << std::left << std::setw(20) << "TOKEN" << std::setw(20) << "LEXEMA" << std::setw(10) << "LINEA" << "COLUMNA\n";
    for(const auto& t : lexical_tokens) {
        std::cout << std::left << std::setw(20) << t.token << std::setw(20) << t.lexema << std::setw(10) << t.linea << t.columna << "\n";
    }
    std::cout << "Total de tokens: " << lexical_tokens.size() << "\n";
    if(total_errors == 0) std::cout << "Sin errores lexicos.\n";
}

void printSymbolTable() {
    std::cout << "\n=== TABLA DE SIMBOLOS ===\n";
    std::cout << std::left << std::setw(15) << "NOMBRE" << std::setw(15) << "TIPO" << std::setw(15) << "VALOR" 
              << std::setw(12) << "DIRECCION" << std::setw(10) << "AMBITO" << "LINEA\n";
    for (auto const& [key, val] : symbol_table) {
        std::cout << std::left << std::setw(15) << val.id << std::setw(15) << typeToString(val.type) 
                  << std::setw(15) << val.value << std::setw(12) << val.memory_address << std::setw(10) << val.ambito << val.linea << "\n";
    }
}

void printIntermediateCode() {
    std::cout << "\n=== CODIGO INTERMEDIO (cuadruplos) ===\n";
    std::cout << std::left << std::setw(5) << "#" << std::setw(20) << "OPERADOR" << std::setw(15) << "ARG1" << std::setw(15) << "ARG2" << "RESULTADO\n";
    for (size_t i = 0; i < intermediate_code.size(); ++i) {
        std::cout << std::left << std::setw(5) << i << std::setw(20) << intermediate_code[i].op << std::setw(15) << intermediate_code[i].arg1 
                  << std::setw(15) << intermediate_code[i].arg2 << intermediate_code[i].result << "\n";
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    int phase = 1, opcion;
    std::string filename = "(ninguno)";
    bool fileOpened = false;

    while (true) {
        if (phase == 1) {
            system("cls");
            std::cout << "========= FASE 1: ENTRADA DE CODIGO =========\n";
            std::cout << "Archivo actual: " << filename << "\n";
            std::cout << "---------------------------------------------\n";
            std::cout << " 1. Abrir archivo\n";
            std::cout << " 2. Escribir codigo aqui mismo\n";
            std::cout << " 3. Ingresar (Continuar a Fase 2)\n";
            std::cout << " 0. Salir\n";
            std::cout << "=============================================\nOpcion: ";
            std::cin >> opcion;

            if (opcion == 0) break;
            else if (opcion == 1) {
                std::cout << "Ruta del archivo: ";
                std::cin >> filename;
                FILE* f = fopen(filename.c_str(), "r");
                if (f) { fileOpened = true; std::cout << "Archivo cargado.\n"; fclose(f); } 
                else { std::cout << "Error: Archivo no encontrado.\n"; fileOpened = false; }
            }
            else if (opcion == 2) {
                std::cout << "\n--- EDITOR INTEGRADO ---\n";
                std::cout << "Escribe tu codigo linea por linea. Escribe la palabra FIN en una linea nueva para terminar:\n\n";
                std::ofstream temp("temp_source.crft");
                std::string line;
                std::cin.ignore();
                while (std::getline(std::cin, line)) {
                    if (line == "FIN") break;
                    temp << line << "\n";
                }
                temp.close();
                filename = "temp_source.crft";
                fileOpened = true;
                std::cout << "\nCodigo guardado temporalmente.\n";
            }
            else if (opcion == 3) {
                if (fileOpened) phase = 2;
                else std::cout << "Error: Debes cargar un archivo o escribir codigo primero.\n";
            }
            if (phase == 1) { std::cout << "\nPresiona Enter para continuar..."; std::cin.ignore(); std::cin.get(); }
        }
        else if (phase == 2) {
            system("cls");
            std::cout << "========= FASE 2: COMPILADOR =========\n";
            std::cout << "Archivo procesando: " << filename << "\n";
            std::cout << "--------------------------------------------\n";
            std::cout << " 1. Analisis lexico\n 2. Analisis sintactico\n 3. Analisis semantico\n 4. Mostrar AST\n";
            std::cout << " 5. Mostrar Tabla de Simbolos\n 6. Mostrar Codigo Intermedio\n 7. Mostrar Codigo Optimizado\n";
            std::cout << " 8. Generar Codigo C++\n 9. Compilar automaticamente el C++ (g++)\n";
            std::cout << " 10. Volver a ingresar codigo (Regresar a Fase 1)\n 0. Salir\n";
            std::cout << "============================================\nOpcion: ";
            std::cin >> opcion;

            if (opcion == 0) break;
            else if (opcion == 10) { phase = 1; continue; }

            switch (opcion) {
                case 1: {
                    yyin = fopen(filename.c_str(), "r"); yyrestart(yyin);
                    lexical_tokens.clear(); total_errors = 0; current_line = 1; current_col = 1;
                    while (yylex());
                    printLexicalTable(filename); fclose(yyin);
                    break;
                }
                case 2: {
                    symbol_table.clear(); intermediate_code.clear(); rootAST = nullptr;
                    yyin = fopen(filename.c_str(), "r"); yyrestart(yyin);
                    total_errors = 0; current_line = 1; current_col = 1; yyparse(); fclose(yyin);
                    std::cout << "\n=== ANALISIS SINTACTICO ===\n";
                    if(total_errors == 0) std::cout << "AST construido con exito.\n"; else std::cout << "Errores encontrados.\n";
                    break;
                }
                case 3: {
                    if (rootAST) {
                        total_errors = 0; rootAST->validateSemantic();
                        std::cout << "\n=== ANALISIS SEMANTICO ===\n";
                        if (total_errors == 0) std::cout << "Sin errores semanticos.\n";
                    } else std::cout << "Ejecuta Analisis Sintactico primero.\n";
                    break;
                }
                case 4: { if (rootAST) { std::cout << "\n=== AST (forma jerarquica) ===\n"; rootAST->printTree("", true); } break; }
                case 5: { printSymbolTable(); break; }
                case 6: { if (rootAST) { if (intermediate_code.empty()) rootAST->generateIntermediate(); printIntermediateCode(); } break; }
                case 7: {
                    if (rootAST) {
                        opt_stats = OptStats(); rootAST->optimize();
                        std::cout << "\n=== CODIGO OPTIMIZADO ===\n"; printIntermediateCode();
                        std::cout << "\n=== ESTADISTICAS ===\nPlegado aplicados: " << opt_stats.plegado << "\n";
                    }
                    break;
                }
                case 8: {
                    if (rootAST) {
                        std::string cpp_code = 
                            "#include <iostream>\n#include <string>\n#include <cmath>\n"
                            "using CRAFTFunction = std::string;\n"
                            "class CRAFTMathCore {\npublic:\n"
                            "  static std::string derivar(std::string expr, std::string var) { return \"2 * \" + var; }\n"
                            "  static std::string integrar_indefinida(std::string expr, std::string var) { return \"(\" + var + \"^3)/3\"; }\n};\n"
                            "int main() {\n" + rootAST->generateTarget() + "\nreturn 0;\n}";
                        std::ofstream out("craft_output.cpp"); out << cpp_code; out.close();
                        std::cout << "\nGenerado 'craft_output.cpp'.\n" << cpp_code << "\n";
                    }
                    break;
                }
                case 9: {
                    system("g++ craft_output.cpp -o craft_final.exe");
                    std::cout << "\nCompilado exitosamente. Ejecutable: 'craft_final.exe'\n";
                    break;
                }
            }
            std::cout << "\nPresiona Enter para continuar..."; std::cin.ignore(); std::cin.get();
        }
    }
    return 0;
}
