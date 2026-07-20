#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

extern int current_line;
extern int current_col;
extern int total_errors;

struct TokenInfo {
    std::string token;
    std::string lexema;
    int linea;
    int columna;
};
extern std::vector<TokenInfo> lexical_tokens;

enum class DataType { ENTERO, FUNCION_MATEMATICA, VOID, ERROR };
inline std::string typeToString(DataType t) {
    if(t == DataType::ENTERO) return "ENTERO";
    if(t == DataType::FUNCION_MATEMATICA) return "FUNCION_MAT";
    return "ERROR";
}

struct Symbol {
    std::string id;
    DataType type;
    std::string value;
    int memory_address;
    int ambito;
    int linea;
    std::string inicializado;
    std::string extra;
};
extern std::unordered_map<std::string, Symbol> symbol_table;
extern int next_memory_address;

struct Quadruple {
    std::string op, arg1, arg2, result;
};
extern std::vector<Quadruple> intermediate_code;
inline std::string newTemp() { static int t = 0; return "t" + std::to_string(t++); }

struct OptStats {
    int plegado = 0, propagacion = 0, simplificacion = 0, temporales = 0, muertos = 0, saltos = 0;
};
extern OptStats opt_stats;

class ASTNode {
public:
    int line, col;
    ASTNode() : line(current_line), col(current_col) {}
    virtual ~ASTNode() = default;
    virtual void printTree(std::string indent, bool isLast) = 0;
    virtual DataType validateSemantic() = 0;
    virtual ASTNode* optimize() { return this; }
    virtual std::string generateIntermediate() = 0;
    virtual std::string generateTarget() = 0;
};

class LiteralNode : public ASTNode {
public:
    std::string val; DataType type;
    LiteralNode(std::string v, DataType t) : val(v), type(t) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << "LITERAL (" << val << ")  [linea " << line << "]\n";
    }
    DataType validateSemantic() override { return type; }
    std::string generateIntermediate() override { return val; }
    std::string generateTarget() override { return val; }
};

class VariableNode : public ASTNode {
public:
    std::string name;
    VariableNode(std::string n) : name(n) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << "IDENTIFICADOR (" << name << ")  [linea " << line << "]\n";
    }
    DataType validateSemantic() override {
        if (symbol_table.find(name) == symbol_table.end()) { total_errors++; return DataType::ERROR; }
        return symbol_table[name].type;
    }
    std::string generateIntermediate() override { return name; }
    std::string generateTarget() override { return name; }
};

class BinaryOpNode : public ASTNode {
public:
    std::string op; ASTNode* left; ASTNode* right;
    BinaryOpNode(std::string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << "OP_BINARIA (" << op << ")  [linea " << line << "]\n";
        std::string newInd = indent + (isLast ? "    " : "|   ");
        if(left) left->printTree(newInd, false);
        if(right) right->printTree(newInd, true);
    }
    DataType validateSemantic() override { return DataType::ENTERO; }
    ASTNode* optimize() override {
        left = left->optimize(); right = right->optimize();
        LiteralNode* litL = dynamic_cast<LiteralNode*>(left);
        LiteralNode* litR = dynamic_cast<LiteralNode*>(right);
        if (litL && litR && litL->type == DataType::ENTERO && litR->type == DataType::ENTERO) {
            opt_stats.plegado++;
            int valL = std::stoi(litL->val), valR = std::stoi(litR->val);
            if (op == "+") return new LiteralNode(std::to_string(valL + valR), DataType::ENTERO);
            if (op == "*") return new LiteralNode(std::to_string(valL * valR), DataType::ENTERO);
        }
        return this;
    }
    std::string generateIntermediate() override {
        std::string t = newTemp();
        intermediate_code.push_back({op, left->generateIntermediate(), right->generateIntermediate(), t});
        return t;
    }
    std::string generateTarget() override { 
        if(op == "^") return "pow(" + left->generateTarget() + ", " + right->generateTarget() + ")";
        return "(" + left->generateTarget() + " " + op + " " + right->generateTarget() + ")"; 
    }
};

class MathNativeNode : public ASTNode {
public:
    std::string op, varFunc, varResp;
    MathNativeNode(std::string o, std::string vf, std::string vr) : op(o), varFunc(vf), varResp(vr) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << "NATIVA (" << op << ")  [linea " << line << "]\n";
        std::string newInd = indent + (isLast ? "    " : "|   ");
        std::cout << newInd << "|-- ID_FUNCION (" << varFunc << ")\n";
        std::cout << newInd << "`-- ID_VARIABLE (" << varResp << ")\n";
    }
    DataType validateSemantic() override { return DataType::FUNCION_MATEMATICA; }
    std::string generateIntermediate() override {
        std::string t = newTemp();
        intermediate_code.push_back({op, varFunc, varResp, t});
        return t;
    }
    std::string generateTarget() override {
        if(op == "derivar") return "CRAFTMathCore::derivar(" + varFunc + ", \"" + varResp + "\")";
        return "CRAFTMathCore::integrar_indefinida(" + varFunc + ", \"" + varResp + "\")";
    }
};

class AssignmentNode : public ASTNode {
public:
    std::string varName; ASTNode* expr;
    AssignmentNode(std::string name, ASTNode* e) : varName(name), expr(e) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << "ASIGNACION (" << varName << ")  [linea " << line << "]\n";
        std::string newInd = indent + (isLast ? "    " : "|   ");
        std::cout << newInd << "|-- ID (" << varName << ")\n";
        expr->printTree(newInd, true);
    }
    DataType validateSemantic() override { return DataType::VOID; }
    ASTNode* optimize() override { expr = expr->optimize(); return this; }
    std::string generateIntermediate() override {
        intermediate_code.push_back({"=", expr->generateIntermediate(), "", varName});
        return varName;
    }
    std::string generateTarget() override { return varName + " = " + expr->generateTarget() + ";"; }
};

class VarDeclNode : public ASTNode {
public:
    DataType type; std::string name;
    VarDeclNode(DataType t, std::string n) : type(t), name(n) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << "DECLARACION (" << name << ")  [linea " << line << "]\n";
        std::cout << indent + (isLast ? "    " : "|   ") << "`-- TIPO (" << typeToString(type) << ")\n";
    }
    DataType validateSemantic() override {
        symbol_table[name] = {name, type, "<entrada>", next_memory_address++, 0, line, "si", ""};
        return DataType::VOID;
    }
    std::string generateIntermediate() override {
        intermediate_code.push_back({"DECLARE", typeToString(type), "", name}); return "";
    }
    std::string generateTarget() override { 
        if(type == DataType::FUNCION_MATEMATICA) return "CRAFTFunction " + name + ";";
        return "int " + name + ";"; 
    }
};

class IONode : public ASTNode {
public:
    std::string mode, varName, extraText;
    IONode(std::string m, std::string v, std::string ex = "") : mode(m), varName(v), extraText(ex) {}
    void printTree(std::string indent, bool isLast) override {
        std::cout << indent << (isLast ? "`-- " : "|-- ") << (mode == "leer" ? "LEER" : "ESCRIBIR") << "  [linea " << line << "]\n";
        std::string newInd = indent + (isLast ? "    " : "|   ");
        if(!extraText.empty()) std::cout << newInd << "|-- CADENA (" << extraText << ")\n";
        std::cout << newInd << "`-- ID (" << varName << ")\n";
    }
    DataType validateSemantic() override { return DataType::VOID; }
    std::string generateIntermediate() override {
        if(mode == "leer") intermediate_code.push_back({"READ", "", "", varName});
        else {
            if(!extraText.empty()) intermediate_code.push_back({"WRITE", "\"" + extraText + "\"", "", ""});
            intermediate_code.push_back({"WRITE", varName, "", ""});
        }
        return "";
    }
    std::string generateTarget() override {
        if (mode == "imprimir") return "std::cout << \"" + extraText + "\" << " + varName + " << std::endl;";
        return "std::cin >> " + varName + ";";
    }
};

class BlockNode : public ASTNode {
public:
    std::vector<ASTNode*> statements;
    void add(ASTNode* stmt) { if(stmt) statements.push_back(stmt); }
    void printTree(std::string indent, bool isLast) override {
        std::cout << "PROGRAMA (programa)  [linea 1, col 1]\n";
        for (size_t i = 0; i < statements.size(); ++i) statements[i]->printTree("", i == statements.size() - 1);
    }
    DataType validateSemantic() override {
        for (auto* s : statements) s->validateSemantic(); return DataType::VOID;
    }
    ASTNode* optimize() override {
        for (size_t i = 0; i < statements.size(); ++i) statements[i] = statements[i]->optimize(); return this;
    }
    std::string generateIntermediate() override {
        intermediate_code.push_back({"PROGRAMA_INICIO", "", "", ""});
        for (auto* s : statements) s->generateIntermediate();
        intermediate_code.push_back({"PROGRAMA_FIN", "", "", ""});
        return "";
    }
    std::string generateTarget() override {
        std::string out = "";
        for (auto* s : statements) out += s->generateTarget() + "\n";
        return out;
    }
};
#endif
