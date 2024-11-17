//landeros cortes pedro jonas 
//compilador v7

#include <iostream>
#include <regex>
#include <fstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>  

using namespace std;

struct Node 
{
    string familia;
    string body;
    unordered_map<string, int> variables;
    vector<unique_ptr<Node>> hijos;

    Node(const string& f, const string& b, const unordered_map<string, int>& v) : familia(f), body(b), variables(v){}

    void AddNode(unique_ptr<Node> hijo) {
        hijos.push_back(move(hijo));
    }
};

map<string, string> families = {
    {"D", R"(int|float|double|char|void)"},                             // tipos de dato
    {"N", R"([a-zA-Z_][a-zA-Z0-9_]*)"},                                 // nombres
    {"S", R"([\+\-\*/%])"},                                             // simbolo
    {"E", R"(=)"},                                                      // igual
    {"NUM", R"(\d+)"},                                                  // numeros
    {"C", R"(\,)"},                                                     // coma
    {"END", R"(;)"},                                                    // punto y coma
    {"K", R"(\{|\})"},                                                  // llaves
    {"F", R"(^\s*(int|float|double|char|void)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{)"}, // encabezado de funciones
    {"IF", R"(^\s*if\s*\(\s*[^)]*\s*\)\s*\{)"},                         // if
    {"FOR", R"(^\s*for\s*\(\s*[^)]*\s*\)\s*\{)"},                       // for
    {"WHILE", R"(^\s*while\s*\(\s*[^)]*\s*\)\s*\{)"},                   // while
    {"INFOR", R"(\s*for\s*\(\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(\d+)\s*;\s*\1\s*[<>]=?\s*\d+\s*;\s*\1\s*(\+\+|--)\s*\))"}, //adentro de un for
    {"VA", R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+)\s*;)"},        // asignación de variables
    {"V", R"(^\s*(int|float|double|char)\s+([a-zA-Z_][a-zA-Z0-9_]*)(\s*=\s*[^,;]*)?(\s*,\s*[a-zA-Z_][a-zA-Z0-9_]*\s*(=\s*[^,;]*)?)*\s*;)"}
};

unordered_map<string, int> functions;

queue<string> SeparateText(string texto)
{
    queue<string> textoSeparado;
    string palabra = "";

    for (char c : texto)
    {
        if (isspace(c))
        {
            if (!palabra.empty())
            {
                textoSeparado.push(palabra);
                palabra = "";
            }
        }
        else if (ispunct(c) && c != '_') 
        {
            if (!palabra.empty())
            {
                textoSeparado.push(palabra);
                palabra = "";
            }
            textoSeparado.push(string(1, c));
        }
        else
        {
            palabra += c;
        }
    }

    if (!palabra.empty())
        textoSeparado.push(palabra);

    return textoSeparado;
}

vector<pair<string, string>> SetTokens(string text)
{
    queue<string> textSeparated = SeparateText(text);
    vector<pair<string, string>> tokens;

    while (!textSeparated.empty())
    {
        string front = textSeparated.front(); 
        textSeparated.pop();
        
        for (const auto &pair : families)
        {
            regex reg(pair.second);
            if (regex_match(front, reg))
            {
                tokens.emplace_back(front, pair.first);
                break;
            }
        }
    }

    return tokens;
}

void CheckVariables(unordered_map<string, int> variables)
{
    for(const auto &pair : variables)
    {
        if(pair.first == "for" || pair.first == "void" || pair.first == "int" || pair.first == "while" || pair.first == "if")
        {
            cout<<"Palabra reservada"<<endl;
            exit(1);
        }
        if(pair.second > 1){
            cout<<"variable "<<pair.first<< " mal declarada o repetida: "<<pair.first<<endl;
            exit(1);
        }
    }
}

void CheckVariables(unordered_map<string, int> variables, string var)
{
    auto it = variables.find(var);
    if(it == variables.end()) 
    {
        cout<<"La variable "<< var <<" no ha sido declarada"<<endl;
        exit(1);
    } 
}

void AddGlobalFunctions(string name)
{
    auto it = functions.find(name);
    if(it != functions.end()) 
    {
        cout<<"Funcion declarada 2 veces: "<<name<<endl;
        exit(2);
    } 
    functions[name]++;
}

void FirstCheck(Node &root, string &code) 
{
    smatch match;
    vector<pair<string, string>> tokens;

    while(!code.empty())
    {
        // Eliminar espacios en blanco al inicio
        code = regex_replace(code, regex(R"(^\s+)"), "");

        bool matched = false;

        //Procesar declaraciones de variables globales primero
        if (regex_search(code, match, regex(families["V"])))
        {
            if (match.position() == 0)  
            {
                tokens = SetTokens(match.str());
                for (const auto& par : tokens) 
                {
                    if (par.second == "N") 
                        root.variables[par.first]++;
                }
                CheckVariables(root.variables);
                code = match.suffix().str();
                matched = true;
            }
        }

        // Procesar funciones
        if (!matched && regex_search(code, match, regex(families["F"])))
        {
            if (match.position() == 0)  
            {
                tokens = SetTokens(match.str());
                AddGlobalFunctions(tokens[1].first);  
                auto functionNode = make_unique<Node>("function", "", root.variables);

                // Revisar parámetros de la función
                string functionSignature = match.str();
                size_t startParams = functionSignature.find('(');
                size_t endParams = functionSignature.find(')');
                
                if (startParams != string::npos && endParams != string::npos && startParams < endParams)
                {
                    string parameters = functionSignature.substr(startParams + 1, endParams - startParams - 1);
                    vector<pair<string, string>> paramParsedTokens = SetTokens(parameters);

                    for (size_t i = 0; i < paramParsedTokens.size(); ++i)
                    {
                        if (paramParsedTokens[i].second == "D" && i + 1 < paramParsedTokens.size() && paramParsedTokens[i + 1].second == "N")
                        {
                            string varName = paramParsedTokens[i + 1].first;
                            functionNode->variables[varName]++;
                            i++;
                        }
                    }
                }

                int braceCount = 1;
                size_t pos = match.position(0) + match.length(0);
                size_t codeSize = code.size();

                while (braceCount > 0 && pos < codeSize)
                {
                    if (code[pos] == '{')
                        braceCount++;
                    else if (code[pos] == '}')
                        braceCount--;
                    pos++;
                }

                if (braceCount != 0)
                {
                    cout << "Llaves desbalanceadas de la funcion " << tokens[1].first << endl;
                    exit(1);
                }

                string bodyContent = code.substr(match.position(0) + match.length(0), pos - (match.position(0) + match.length(0)) - 1);

                functionNode->body = bodyContent;
                root.AddNode(move(functionNode));
                code = code.substr(pos);
                matched = true;
            }
        }

        //si no coincide, terminar el programa
        if (!matched)
        {
            if (!code.empty())
            {
                cout << "codigo incorrecto en la declaracion global: " << code << endl;
                exit(1);
            }
        }
    }

    code.erase(remove_if(code.begin(), code.end(), ::isspace), code.end());
    if (!code.empty()) 
    {
        cout << "codigo incorrecto" << endl;
        exit(1);
    }
}

void CheckNode(Node &nodo)
{
    smatch match;
    vector<pair<string, string>> tokens;

    string code = nodo.body;

    while(!code.empty())
    {
        string oldCode = code;

        // Eliminar espacios en blanco al inicio
        code = regex_replace(code, regex(R"(^\s+)"), "");

        bool matched = false;

        // Procesar condicionales
        if (regex_search(code, match, regex(families["IF"])))
        {
            if (match.position() == 0)
            {
                tokens = SetTokens(match.str());
                auto ifNode = make_unique<Node>("if", "", nodo.variables);

                // Revisar condición del if
                string ifSignature = match.str();
                size_t startParams = ifSignature.find('(');
                size_t endParams = ifSignature.find(')');

                if (startParams != string::npos && endParams != string::npos && startParams < endParams)
                {
                    string parameters = ifSignature.substr(startParams + 1, endParams - startParams - 1);
                    vector<pair<string, string>> paramParsedTokens = SetTokens(parameters);

                    for (const auto &paramToken : paramParsedTokens)
                    {
                        if (paramToken.second == "N") 
                            CheckVariables(nodo.variables, paramToken.first);
                    }
                }

                // Procesar el cuerpo del if
                int braceCount = 1;
                size_t pos = match.position(0) + match.length(0);
                size_t codeSize = code.size();

                while (braceCount > 0 && pos < codeSize)
                {
                    if (code[pos] == '{')
                        braceCount++;
                    else if (code[pos] == '}')
                        braceCount--;
                    pos++;
                }

                if (braceCount != 0)
                {
                    cout << "Llaves desbalanceadas en el if" << endl;
                    exit(1);
                }

                string bodyContent = code.substr(match.position(0) + match.length(0), pos - (match.position(0) + match.length(0)) - 1);
                ifNode->body = bodyContent;

                CheckNode(*ifNode);

                nodo.AddNode(move(ifNode));

                code = code.substr(pos);
                matched = true;
                continue;
            }
        }

        // Procesar bucles for y while
        if (!matched && (regex_search(code, match, regex(families["FOR"])) || regex_search(code, match, regex(families["WHILE"]))))
        {
            if (match.position() == 0)
            {
                tokens = SetTokens(match.str());
                auto loopNode = make_unique<Node>("loop", "", nodo.variables);   

                // Revisar condición del bucle
                string loopSignature = match.str();
                size_t startParams = loopSignature.find('(');
                size_t endParams = loopSignature.find(')');

                if (startParams != string::npos && endParams != string::npos && startParams < endParams)
                {
                    string parameters = loopSignature.substr(startParams + 1, endParams - startParams - 1);
                    vector<pair<string, string>> paramParsedTokens = SetTokens(parameters);
                    for (const auto &paramToken : paramParsedTokens)
                    {
                        if (paramToken.second == "N") 
                            CheckVariables(nodo.variables, paramToken.first);
                    }
                }

                // Procesar el cuerpo del bucle
                int braceCount = 1;
                size_t pos = match.position(0) + match.length(0);
                size_t codeSize = code.size();

                while (braceCount > 0 && pos < codeSize)
                {
                    if (code[pos] == '{')
                        braceCount++;
                    else if (code[pos] == '}')
                        braceCount--;
                    pos++;
                }

                if (braceCount != 0)
                {
                    cout << "Llaves desbalanceadas en el bucle" << endl;
                    exit(1);
                }

                string bodyContent = code.substr(match.position(0) + match.length(0), pos - (match.position(0) + match.length(0)) - 1);
                loopNode->body = bodyContent;

                CheckNode(*loopNode);

                nodo.AddNode(move(loopNode));

                code = code.substr(pos);
                matched = true;
                continue;
            }
        }

        // Procesar declaraciones de variables locales
        if (!matched && regex_search(code, match, regex(families["V"])))
        {
            if (match.position() == 0)
            {
                tokens = SetTokens(match.str());
                string anterior = "";
                string actual = "";
                for (const auto& par : tokens) 
                {
                    actual = par.second;
                    //solo se deberia de guardar si estra atras de una coma o atras de un tipo de dato
                    if (par.second == "N" && (anterior == "C" || anterior == "D" )) 
                        nodo.variables[par.first]++;
                    
                    anterior = actual;
                }
                CheckVariables(nodo.variables);
                code = match.suffix().str();
                matched = true;
                continue;
            }
        }

        // Procesar asignaciones de variables
        if (!matched && regex_search(code, match, regex(families["VA"])))
        {
            if (match.position() == 0)
            {
                tokens = SetTokens(match.str());
                for (const auto& par : tokens) 
                {
                    if (par.second == "N") 
                        CheckVariables(nodo.variables, par.first);
                }
                code = match.suffix().str();
                matched = true;
                continue;
            }
        }

        // Si no se pudo procesar nada y el código no está vacío, reportar error
        if (!matched)
        {
            if (!code.empty())
            {
                cout << "Codigo incorrecto en el nodo " << nodo.familia << ": " << code << endl;
                exit(1);
            }
        }
    }

    code.erase(remove_if(code.begin(), code.end(), ::isspace), code.end());
    if (!code.empty()) 
    {
        cout << "Codigo incorrecto en el nodo " << nodo.familia << endl;
        exit(1);
    }
}

void PrintMap(unordered_map<string, int> map) {
    for (const auto& pair : map) {
        cout << "Variable: " << pair.first << ", Frecuencia: " << pair.second << endl;
    }
}

void PrintChildren(const Node& root) {
    cout << "Hijos del nodo " << root.familia << ":" << endl;
    for (const auto& hijo : root.hijos) {
        cout << "Familia: " << hijo->familia << endl;
        cout << "Variables locales de este nodo:" << endl;
        PrintMap(hijo->variables);
        cout << "-----------------------------------" << endl;
    }
}

unique_ptr<Node> InitiTree() {
    return make_unique<Node>("root", "", unordered_map<string, int>());
}

int main(int argc, char const *argv[])
{
    fstream archivo("codigo.c");
    string linea, singleLineText;
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            singleLineText += linea + " ";
        }
        archivo.close();
        cout << endl;
    } else {
        cout << "Archivo faltante" << endl;
        return 1;
    }
    auto root = InitiTree();
    // Identificar funciones y variables globales en el nodo raíz
    FirstCheck(*root, singleLineText);
    
    cout<<"Variables globales"<<endl;
    PrintMap(root->variables);

    cout<<"Funciones"<<endl;
    PrintChildren(*root);

    cout << "Verificando nodos hijos:" << endl;
    for (const auto& child : root->hijos) {
        CheckNode(*child);
    }

    cout << "Analisis completado exitosamente." << endl;

    return 0;
}