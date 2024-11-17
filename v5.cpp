#include <iostream>
#include <regex>
#include <fstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include<queue>

using namespace std;

unordered_map<string, int> globalVariables;
unordered_map<string, int> functions;

map<string, string> families = {
    {"D", R"(int|float|double|char)"},                             // tipos de dato (data types)
    {"N", R"([a-zA-Z_][a-zA-Z0-9_]*)"},                            // nombres (names)
    {"S", R"([\+\-\*/%])"},                                        // símbolos (symbol)
    {"E", R"(=)"},                                                 // igual (equal)
    {"NUM", R"(\d+)"},                                             // números (numbers)
    {"C", R"(,)"},                                                 // coma
    {"END", R"(;)"},
    {"K", R"(\{|\})"},                                             // Llaves
    {"F", R"((int|float|double|char|void)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{[^}]*\})"}, // funciones
    {"VG", R"((int|float|double|char)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*(=\s*[^;]+)?\s*;)"} // variables globales
};



queue<string> SeparateText(string texto)
{
    queue<string> textoSeparado;
    string palabra = "";

    for (char c : texto)
    {
        if (isspace(c))
        {
            if (palabra != "")
            {
                textoSeparado.push(palabra);
                palabra = "";
            }
        }
        else if (ispunct(c) && c != '_') // simbolos diferentes de _
        {
            if (palabra != "")
            {
                textoSeparado.push(palabra);
                palabra = "";
            }
            textoSeparado.push(string(1, c));
        }
        else
            palabra += c;
    }

    if (palabra != "")
        textoSeparado.push(palabra);

    return textoSeparado;
}

void Printmap(unordered_map<string, string> map)
{
    for (const auto& pair : map) {
        std::cout << "Clave: " << pair.first << ", Valor: " << pair.second << std::endl;
    }
}

unordered_map<string, string> SetTokens(string text)
{
    queue<string> textSeparated = SeparateText(text);
    unordered_map<string, string> tokens;

    while (!textSeparated.empty())
    {
        string front = textSeparated.front(); 
        textSeparated.pop();
        
        for (const auto &pair : families)
        {
            regex reg(pair.second);
            if (regex_match(front, reg))
            {
                tokens[front] = pair.second;
                break;
            }
        }
    }

    return tokens;
}

struct Node 
{
    string familia;
    string body;
    unordered_map<string, int> variables;
    vector<unique_ptr<Node>> hijos;

    Node(const string& f, const string& b) : familia(f), body(b) {}

    void AddNode(unique_ptr<Node> hijo) {
        hijos.push_back(move(hijo));
    }
};

void CheckLocalVariables(Node &node)
{
    //revisar si la frecuencia de alguna variable es mayor a 1
    for (const auto &pair : node.variables)
    {
        if(pair.second > 1)
        {
            cout<<"La variable "<<pair.first<<"ya habia sido declarada"<<endl;
            exit(1);
        }
    }
    
}

void AddGlobalFuncionts(string name)
{
    auto it = functions.find(name);
    if(it != functions.end()) 
    {
        cout<<"Nombre de funcion previamente declarado"<<endl;
        exit(2);
    } 
    functions[name]++;
}

void AddGlobalVariables(string variable)
{
    auto it = globalVariables.find(variable);
    if(it != globalVariables.end()) 
    {
        cout<<"variable global previamente declarada"<<endl;
        exit(2);
    } 
    globalVariables[variable]++;
}

void CheckVariableAssignment(string variable)
{
    auto it = globalVariables.find(variable);
    if(it == globalVariables.end()) 
    {
        cout<<"La variable "<< variable <<"no ha sido declarada"<<endl;
        exit(2);
    } 
}

void CheckBraces(queue<string> bracesLine)
{
    int braces = 1;
    string word = ""; 

    while(!bracesLine.empty())
    {
        word = bracesLine.front();
        bracesLine.pop();
        if(word == "{") braces++;
        if(word == "}") braces--;
    }

    if(braces != 0)
    {
        cout<<"LLaves no balanceadas"<<endl;
        exit(2);
    }
}

void AddBody(queue<string> lines, Node &node)
{
    //se va a pasar una queue que va a tener todas las palabras
    //se comienza despues de la primer llave {, se va a agregar todo y luego se hara un conteo de llaves para ver si esta balanceado 
    string aux = "";
    queue<string> braces;

    while(!lines.empty())
    {
        if(lines.front() == "{" || lines.front() == "}")
        {
            braces.push(lines.front());
        }
        aux += lines.front();
        lines.pop();
    }

    CheckBraces(braces);
    node.body = aux;
}

void IdentifyFunctions(Node &root, const string& code) {
    smatch match;
    string remainingCode = code;

    // Buscar variables globales en el código
    while (regex_search(remainingCode, match, regex(families["VG"]))) {
        // Crear un nodo de variable global
        auto variableNode = make_unique<Node>("global_var", match.str());
        root.AddNode(move(variableNode));
        remainingCode = match.suffix().str();
    }

    remainingCode = code; // Resetear remainingCode para buscar variables desde el inicio

    // Buscar funciones en el código
    while (regex_search(remainingCode, match, regex(families["F"]))) {
        // Crear un nodo de función
        auto functionNode = make_unique<Node>("function", match.str());
        root.AddNode(move(functionNode));
        remainingCode = match.suffix().str();
    }
}


void CheckNode(Node &nodo)
{
    //declaracion de expresiones para los esqueletos 
    regex functionRegex(R"((int|float|double|char)\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(\s*\)\s*\{)");
    regex loopRegex(R"(for\s*\(.*?\)\s*\{)|while\s*\(.*?\)\s*\{)");
    regex ifRegex(R"(if\s*\(.*?\)\s*\{)");
    regex varDeclarationRegex(R"((int|float|double|char)\s+[a-zA-Z_][a-zA-Z0-9_]*\s*;)");
    regex assignmentRegex(R"([a-zA-Z_][a-zA-Z0-9_]*\s*=\s*[^;]+;)");

    smatch match;

    //verificar que no haya funciones declaradas dentro de funciones
    if (nodo.familia == "function") {
        for (const auto& hijo : nodo.hijos) {
            if (hijo->familia == "function") {
                cout << "Hay una funcion dentro de una funcion" << endl;
                exit(1);
            }
        }
    }

    if (regex_search(nodo.body, match, functionRegex))
    {
        string funcionBody = match.suffix().str();
        while (regex_search(funcionBody, match, loopRegex) || regex_search(funcionBody, match, ifRegex))
        {
            //si encuentra un bucle
            if (regex_search(funcionBody, match, loopRegex))
            {
                auto loopNode = make_unique<Node>("loop", match.str());
                nodo.AddNode(move(loopNode));
                funcionBody = match.suffix().str();
            }
            //si encuentra una condicional
            if(regex_search(funcionBody, match, ifRegex))
            {
                auto ifNode = make_unique<Node>("conditional", match.str());
                nodo.AddNode(move(ifNode));
                funcionBody = match.suffix().str();
            }
        }

        string remainingBody = funcionBody;
        while (regex_search(remainingBody, match, varDeclarationRegex) || regex_search(remainingBody, match, assignmentRegex)) {
            //decalaracion de una variable dentro de la funcion
            if (regex_search(remainingBody, match, varDeclarationRegex)) {
                nodo.variables[match.str()]++;
                remainingBody = match.suffix().str();
            } else if (regex_search(remainingBody, match, assignmentRegex)) { //asignacion de una variable dentro de la funcion
                cout << "Asignación de variable detectada: " << match.str() << endl;
                remainingBody = match.suffix().str();
            }
        }     
    }

    //repetir lo mismo con todos los hijos del nodo
    for (auto& hijo : nodo.hijos) {
        CheckNode(*hijo);
    }
}

unique_ptr<Node> InitiTree() {
    return make_unique<Node>("root", "");
}

// Función para imprimir el árbol desde un nodo dado, con indentación para mostrar la jerarquía
void PrintTree(const Node& nodo, int nivel = 0) {
    // Imprimir la información del nodo con indentación según el nivel
    string indent(nivel * 2, ' ');  // Crear sangría según el nivel
    cout << indent << "Nodo: " << nodo.familia << ", Cuerpo: " << nodo.body << endl;

    // Imprimir variables declaradas en el nodo actual (si las hay)
    if (!nodo.variables.empty()) {
        cout << indent << "  Variables:\n";
        for (const auto& var : nodo.variables) {
            cout << indent << "    " << var.first << " (declarada " << var.second << " vez/veces)" << endl;
        }
    }

    // Llamada recursiva para imprimir cada hijo con un nivel de indentación mayor
    for (const auto& hijo : nodo.hijos) {
        PrintTree(*hijo, nivel + 1);
    }
}


int main(int argc, char const *argv[])
{
    unordered_map<string, string> tokens;

    fstream archivo("codigo.c");
    string linea, singleLineText;
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            singleLineText += linea + " ";
        }
        archivo.close();
        tokens = SetTokens(singleLineText);
        cout << endl;
    } else {
        cout << "Archivo faltante" << endl;
    }

    auto root = InitiTree();

    // Identificar funciones y variables globales en el nodo raíz
    IdentifyFunctions(*root, singleLineText);

    // Procesar el árbol desde el nodo raíz
    //CheckNode(*root);

    PrintTree(*root);

    return 0;
}