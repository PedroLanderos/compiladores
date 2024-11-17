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

// Almacena la frecuencia de la declaración de variables globales.
unordered_map<string, int> variables;
unordered_map<string, int> functions;

// Definir el mapa de familias de patrones de expresión regular
map<string, string> families = {
    {"D", R"(int|float|double|char)\\s+"}, // tipos de dato (data types)
    {"N", R"([a-zA-Z_][a-zA-Z0-9_]*)"},     // nombres (names)
    {"S", R"([\+\-\*/%])"},                  // símbolos (symbol)
    {"E", R"(=)"},                           // igual (equal)
    {"NUM", R"(\d+)"},                       // números (numbers)
    {"C", R"(,)"},                           // coma
    {"END", R"(;)"},
    {"K", R"({|})"},    
    {"F", R"((int|float|double|char)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{\s*\})"}, // funciones con cuerpo vacío o contenido
     
    {"IF", R"(if\s*\(.*?\)\s*\{\s*.*?\s*\})"},//if normal
    {"FOR", R"(for\s*\(\s*[^;]*;\s*[^;]*;\s*[^)]*\)\s*\{\s*.*?\s*\})"} //for
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


//Deifne los nodos del arbol
class FunctionNode {
public:
    string tipo;      
    string cuerpo;        
    vector<shared_ptr<FunctionNode>> hijos; // nodos hijos para almacenar más tipos de datos 

    // constructor
    FunctionNode(const string& tipo, const string& cuerpo)
        : tipo(tipo), cuerpo(cuerpo) {}

    void AddNode(shared_ptr<FunctionNode> hijo) {
        hijos.push_back(hijo);
    }
    
    // método que imprime los nodos en forma organizada
    void PrintNode(int nivel = 0) const {
        cout << string(nivel * 2, ' ') << "- " << tipo << ": " << cuerpo << endl;
        for (const auto& hijo : hijos) {
            hijo->PrintNode(nivel + 1);
        }
    }
};

class MainTree {
public:
    shared_ptr<FunctionNode> raiz;

    // constructor
    MainTree() {
        raiz = make_shared<FunctionNode>("Principal", "");  // El nodo raíz es main
    }

    //Llamar a la funcion que agrega nodos para crear los nodos del arbol
    void Add(const string& tipo, const string& cuerpo) {
        raiz->AddNode(make_shared<FunctionNode>(tipo, cuerpo));
    }

    //imprimir todo el árbol completo llamando a cada uno de los nodos de forma recursiva
    void PrintTree() const {
        if (raiz) {
            raiz->PrintNode();
        }
    }
};

// Funciones para analizar diferentes estructuras
void AddDeclarations(const string& body, shared_ptr<FunctionNode> parent) {
    regex decRegex(families["D"]);  
    smatch match;
    string::const_iterator searchStart(body.cbegin());
    
    while (regex_search(searchStart, body.cend(), match, decRegex)) {
        string tipo = "D";
        string cuerpo = match[0]; 
        parent->AddNode(make_shared<FunctionNode>(tipo, cuerpo));
        searchStart = match.suffix().first; // Avanza la búsqueda
    }
}

void AddControlStructures(const string& body, shared_ptr<FunctionNode> parent) {
    regex ifElseRegex(families["IF"]);
    regex forRegex(families["FOR"]);
    regex whileRegex(families["WHILE"]);
    regex doWhileRegex(families["DO_WHILE"]);
    smatch match;
    string::const_iterator searchStart(body.cbegin());

    // Buscar if-else
    while (regex_search(searchStart, body.cend(), match, ifElseRegex)) {
        string tipo = "IF";
        string cuerpo = match[0]; 
        parent->AddNode(make_shared<FunctionNode>(tipo, cuerpo));
        searchStart = match.suffix().first; // Avanza la búsqueda
    }

    // Reiniciar búsqueda para otras estructuras de control
    searchStart = body.cbegin();

    // Buscar for
    while (regex_search(searchStart, body.cend(), match, forRegex)) {
        string tipo = "FOR";
        string cuerpo = match[0]; 
        parent->AddNode(make_shared<FunctionNode>(tipo, cuerpo));
        searchStart = match.suffix().first; // Avanza la búsqueda
    }

    // Reiniciar búsqueda para otras estructuras de control
    searchStart = body.cbegin();

    // Buscar while
    while (regex_search(searchStart, body.cend(), match, whileRegex)) {
        string tipo = "WHILE";
        string cuerpo = match[0]; 
        parent->AddNode(make_shared<FunctionNode>(tipo, cuerpo));
        searchStart = match.suffix().first; // Avanza la búsqueda
    }

    // Reiniciar búsqueda para otras estructuras de control
    searchStart = body.cbegin();

    // Buscar do-while
    while (regex_search(searchStart, body.cend(), match, doWhileRegex)) {
        string tipo = "DO_WHILE";
        string cuerpo = match[0]; 
        parent->AddNode(make_shared<FunctionNode>(tipo, cuerpo));
        searchStart = match.suffix().first; // Avanza la búsqueda
    }
}

void AnalyzeFunctionBody(const string& cuerpo, shared_ptr<FunctionNode> parent) {
    if (!cuerpo.empty()) {  // Verificar si el cuerpo de la función no está vacío
        AddDeclarations(cuerpo, parent);
        AddControlStructures(cuerpo, parent);
    }
}

//separa el programa en las funciones principales
void AddFunctions(const string& codigo, MainTree& arbolFunciones) {
    regex funcionRegex(families["F"]);  
    smatch match;

    string::const_iterator searchStart(codigo.cbegin());
    while (regex_search(searchStart, codigo.cend(), match, funcionRegex)) {
        string tipo = "F";
        string cuerpo = match[0]; 

        size_t posOpen = cuerpo.find("{");
        size_t posClose = cuerpo.rfind("}");
        /*if (posOpen == string::npos || posClose == string::npos || posClose <= posOpen + 1) {
            cout << "Error: La función está mal escrita o incompleta.\n";
            exit(EXIT_FAILURE);  // Terminar el programa si la función está mal escrita
        }*/
        auto funcionNode = make_shared<FunctionNode>(tipo, cuerpo);
        arbolFunciones.Add(tipo, cuerpo);



        // Extraer el cuerpo de la función para analizarlo
        /*size_t posOpen = cuerpo.find("{");
        size_t posClose = cuerpo.rfind("}");
        if (posOpen != string::npos && posClose != string::npos && posClose > posOpen + 1) {
            string body = cuerpo.substr(posOpen + 1, posClose - posOpen - 1); // Obtener solo el cuerpo entre llaves
            AnalyzeFunctionBody(body, funcionNode);
        }
        */
        searchStart = match.suffix().first; 
    }
}

void InitTree(MainTree& mainTree) {
    // Leer el archivo y almacenar el contenido en una sola cadena
    fstream archivo("codigo.c");
    string linea, codigoCompleto;
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            codigoCompleto += linea + " ";
        }
        archivo.close();
    } else {
        cout << "Archivo faltante" << endl;
        return;
    }

    cout<<"el texto fue procesado"<<endl;

    // Llamar a AddFunctions para agregar funciones al árbol
    AddFunctions(codigoCompleto, mainTree);
}

void ChekBraces(char word)
{
    //se asume que algun esqueleto de funciones, bucles, condicionales es correcto y se hace un conteo de llaves
    //for() { 
    int keys = 1;

    if(word == '{') keys++;

    if(word == '}') keys--;

    if (keys != 0)
    {
        cout<<"falta de llaves"<<endl;
        exit(1);
    }
}

int main() {
    MainTree arbol;

    // Inicializar el árbol con el contenido del archivo
    InitTree(arbol);

    // Imprimir el árbol de funciones
    cout << "arbol:\n";
    arbol.PrintTree();

    return 0;
}


/*
void resta()
{

}

void suma()
{

}

int main()
{   

}


*/