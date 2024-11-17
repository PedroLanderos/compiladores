//landeros cortes pedro jonas 
//compilador v10
/*
Cambios:
1. Actualizacion la estructura nodo para que las variables contengan su tipo de dato.
1.1 Actualizacion de las funciones que utilizan el map de variables de los nodos.

2. Actualizacion del map de funciones para que contenga el tipo de retorno. 
2.1 Actualizacion de las funciones que utilizan el map de funciones

3. Actualizacion para la verificacion de los parentesis de un bucle for, while y condicionales if.

*/
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

//Estructura base del nodo
struct Node 
{
    string familia;
    string body;
    unordered_map<string, pair<string, int>> variables;
    vector<unique_ptr<Node>> hijos;

    Node(const string& f, const string& b, const unordered_map<string, pair<string, int>>& v) : familia(f), body(b), variables(v){}

    void AddNode(unique_ptr<Node> hijo) {
        hijos.push_back(move(hijo));
    }
};

//Expresiones regulares para la deteccion de tokens.
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
    {"VA", R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+)\s*;)"},        // asignación de variables
    {"V", R"(^\s*(int|float|double|char)\s+([a-zA-Z_][a-zA-Z0-9_]*)(\s*=\s*[^,;]*)?(\s*,\s*[a-zA-Z_][a-zA-Z0-9_]*\s*(=\s*[^,;]*)?)*\s*;)"}
};

//Map que contiene a las funciones, sus tipos de datos y tipo de retorno.
unordered_map<string, pair<string, int>> functions;

//Funcion para separar un string en palabras.
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

//Funcion para detectar los tokens de cada palabra y almacenarlo en un map con el token y el valor.
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

//Revisar una variable ha sido mal declarada o esta repetida.
void CheckVariables(unordered_map<string, pair<string, int>> variables)
{
    for(const auto &pair : variables)
    {
        if(pair.first == "for" || pair.first == "void" || pair.first == "int" || pair.first == "while" || pair.first == "if")
        {
            cout<<"Palabra reservada"<<endl;
            exit(1);
        }
        if(pair.second.second > 1){
            cout<<"variable mal declarada o repetida: "<<pair.first<<endl;
            exit(1);
        }
    }
}

//Revisar si una variable no ha sido asignada.
void CheckVariables(unordered_map<string, pair<string, int>> variables, string var)
{
    auto it = variables.find(var);
    if(it == variables.end()) 
    {
        cout<<"La variable "<< var <<" no ha sido declarada"<<endl;
        exit(1);
    } 
}

//Agregar la informacion de la funcion al map que contiene los esqueletos de funciones.
void AddGlobalFunctions(string name, string returnType)
{
    auto it = functions.find(name);
    if(it != functions.end()) 
    {
        cout<<"Funcion declarada 2 veces: "<<name<<endl;
        exit(2);
    } 
    functions[name].first = returnType;
    functions[name].second++;
}

//Separar el codigo en funciones. Guardar las variables globales al nodo root.
void CreateATSTree(Node &root, string &code) 
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
            //se deben agregar el nombre de las variables y el tipo 
            if (match.position() == 0)  
            {
                string tipo = "";
                tokens = SetTokens(match.str());
                for (const auto& par : tokens) 
                {
                    if(par.second == "D")
                        tipo = par.first;
                        
                    if (par.second == "N") 
                        root.variables[par.first] = {tipo, root.variables[par.first].second + 1};
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
                AddGlobalFunctions(tokens[1].first, tokens[0].first);  
                auto functionNode = make_unique<Node>("function", "", root.variables);

                //revisar parametros de la funcion
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
                            string varType = paramParsedTokens[i].first;

                            functionNode->variables[varName] = {varType, functionNode->variables[varName].second + 1};

                        }
                        CheckVariables(functionNode->variables);
                        i++;
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

//Revisa si las condiciones son correctas
void ValidateCondition(const string& condition) 
{
    regex validConditionRegex(R"((\d+|\w+)\s*(==|>=|<=|>|<)\s*(\d+|\w+)(\s*(&&|\|\|)\s*(\d+|\w+)\s*(==|>=|<=|>|<)\s*(\d+|\w+))*)");
    smatch match;
    if (!regex_match(condition, match, validConditionRegex)) 
    {
        cout<<"Condicion invalida"<<endl;
        exit(1);
    }
}

void ValidateFor(const string& condition)
{
    regex validFor(R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(-?\d+)\s*;\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*(<=|>=|<|>|==)\s*(-?\d+)\s*;\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*(\+\+|--)\s*$)");
    smatch match;

    if(!regex_match(condition, match, validFor))
    {
        cout<<"Condicion invalida"<<endl;
        exit(1);
    }
}

//Revisar recurisvamente las funciones para encontrar bucles, condicionales y declaraciones de variables.
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
                    ValidateCondition(parameters);
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
                string matchedStr = match.str();

                tokens = SetTokens(match.str());
                auto loopNode = make_unique<Node>("loop", "", nodo.variables);   

                //revisar los parentesis del bucle
                string loopSignature = match.str();
                size_t startParams = loopSignature.find('(');
                size_t endParams = loopSignature.find(')');

                if (loopSignature.substr(0, 5) == "while") 
                {
                    if (startParams != string::npos && endParams != string::npos && startParams < endParams)
                    {
                        string condition = loopSignature.substr(startParams + 1, endParams - startParams - 1);
                        ValidateCondition(condition);  
                    }
                }

                if (loopSignature.substr(0, 3) == "for") 
                {
                    if (startParams != string::npos && endParams != string::npos && startParams < endParams)
                    {
                        string condition = loopSignature.substr(startParams + 1, endParams - startParams - 1);
                        ValidateFor(condition);  
                    }
                }

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
                string tipo = "";
                tokens = SetTokens(match.str());
                string anterior = "";
                string actual = "";
                for (const auto& par : tokens) 
                {
                    if(par.second == "D")
                        tipo = par.first;
                    actual = par.second;
                    //solo se deberia de guardar si estra atras de una coma o atras de un tipo de dato
                    if (par.second == "N" && (anterior == "C" || anterior == "D" )) 
                    {
                        nodo.variables[par.first] = nodo.variables.count(par.first) 
                                           ? make_pair(tipo, nodo.variables[par.first].second + 1) 
                                           : make_pair(tipo, 1);  
                    }

                    if(par.second == "N")
                        CheckVariables(nodo.variables, par.first);

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

//Imprimir un map y sus valores.
void PrintMap(const unordered_map<string, pair<string, int>>& map) {
    for (const auto& pair : map) {
        cout << "Hash: " << pair.first 
             << ", Tipo: " << pair.second.first 
             << ", Frecuencia: " << pair.second.second 
             << endl;
    }
}

//Imprimir la informacion de los nodos
void PrintChildren(const Node& root) {
    cout << "Hijos del nodo " << root.familia << ":" << endl;

    // Iterar sobre cada hijo del nodo
    for (const auto& hijo : root.hijos) {
        cout << "Familia: " << hijo->familia << endl;

        // Imprimir las variables locales de este nodo (hijo)
        cout << "Variables locales de este nodo:" << endl;
        PrintMap(hijo->variables);  // Llamamos a PrintMap para imprimir las variables del hijo
        cout << "-----------------------------------" << endl;
    }
}

//Inicializar el nodo root vacio. 
unique_ptr<Node> InitiTree() {
    return make_unique<Node>("root", "", unordered_map<string, pair<string, int>>());
}

//Funcion principal del programa que contiene los arboles y el codigo a revisar.
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
    //Crear nodos de funciones e identificar variables globales
    CreateATSTree(*root, singleLineText);
    
    cout<<"Funciones"<<endl;
    PrintMap(functions);

    cout << "Verificando nodos hijos:" << endl;
    for (const auto& child : root->hijos) {
        CheckNode(*child);
        PrintChildren(*child);
    }

    cout << "Analisis completado exitosamente." << endl;

    return 0;
}