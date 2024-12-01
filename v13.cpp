//landeros cortes pedro jonas 
//compilador v13
/*
Cambios:
1. Arreglo de bug en la funcion checknode para detectar el numero correcto de parametros
2. Cambio de tipo apuntador a vector a vector en estructura Node.
3. validacion de asignacion de tipo de dato correcto 

4. Resolucion de bug !=, &&, || en condicionales

5. Actualizacion de forma for para:
(asignacion de varaible ; condicional ; incremento)

6. actualizacion de familia de v para detectar 'c' y 2.2 (char y decimal)

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
    string nombreDeFuncion;
    string familia;
    string body;
    unordered_map<string, pair<string, int>> variables;
    vector<unique_ptr<Node>> hijos;
    vector<string> tiposDeParametos;

    Node(const string& f, const string& b,  const unordered_map<string, pair<string, int>>& v, const string &n) : familia(f), body(b), variables(v), nombreDeFuncion(n){}

    void AddNode(unique_ptr<Node> hijo) {
        hijos.push_back(move(hijo));
    }
};

//Expresiones regulares para la deteccion de tokens.
map<string, string> families = {
    {"D", R"(int|double|char|void)"},                             // tipos de dato
    {"N", R"(^[a-zA-Z_][a-zA-Z0-9_]*)"},                                 // nombres
    {"S", R"([\+\-\*/%])"},                                             // simbolo
    {"E", R"(=)"},                                                      // igual
    {"NUM", R"(((\d+(\.\d+)?|'[a-zA-Z]')))"},                                                  // numeros
    {"C", R"(\,)"},                                                     // coma
    {"END", R"(;)"},                                                    // punto y coma
    {"K", R"(\{|\})"},                                                  // llaves
    {"F", R"(^\s*(int|float|double|char|void)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{)"}, // encabezado de funciones
    {"IF", R"(^\s*if\s*\(\s*[^)]*\s*\)\s*\{)"},                         // if
    {"FOR", R"(^\s*for\s*\(\s*[^)]*\s*\)\s*\{)"},                       // for
    {"WHILE", R"(^\s*while\s*\(\s*[^)]*\s*\)\s*\{)"},                   // while
    {"VA", R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+)\s*;)"},        // asignación de variables
    {"R", R"(\s*return\s+([a-zA-Z_]\w*|\d+|'[a-zA-Z]'|\d+\.\d+)\s*;)"},
    {"CALL", R"(^\s*(\b\w+\s+\w+\s*=\s*)?\b\w+\s*\([^;]*\)\s*;)"},
    {"V", R"(^\s*(int|float|double|char)\s+([a-zA-Z_][a-zA-Z0-9_]*)(\s*=\s*'([a-zA-Z])'|\s*=\s*[^,;]*)?(\s*,\s*[a-zA-Z_][a-zA-Z0-9_]*\s*(=\s*'([a-zA-Z])'|\s*=\s*[^,;]*)?)*\s*;)"}
};

//Map que contiene a las funciones, sus tipos de datos y tipo de retorno.
unordered_map<string, pair<string, int>> functions;

vector<string> reservedWords = {
        "int", "float", "char", "return", "while", "for", "if", "main"
    };

void CheckReservedWords(string var)
{
    auto it = find(reservedWords.begin(), reservedWords.end(), var);
    if(it != reservedWords.end())
    {
        cout<<"No se puede nombrar algo con una palabra reservada"<<endl;
        exit(1);
    }
}

void PrintTokens(const vector<pair<string, string>>& tokens) {
    for (const auto& token : tokens) {
        cout << "Token: " << token.first << ", Tipo: " << token.second << endl;
    }
}

void PrintQueue(queue<string> q)
{
    while(!q.empty())
    {
        cout<<q.front()<<endl;
        q.pop();
    }
    
}

//Funcion para separar un string en palabras.
queue<string> SeparateText(string texto)
{
    queue<string> textoSeparado;
    string palabra = "";
    bool enComillas = false; 

    //double a = 2.2;

    for (char c : texto)
    {
        // Manejo de comillas simples para los caracteres literales
        if (c == '\'' && !enComillas)
        {
            enComillas = true;
            palabra += c;  
        }
        else if (c == '\'' && enComillas)  
        {
            enComillas = false;
            palabra += c;  
            textoSeparado.push(palabra);  
            palabra = "";  
        }
        // Manejo de espacios
        else if (isspace(c)) 
        {
            if (!palabra.empty())
            {
                textoSeparado.push(palabra);
                palabra = "";
            }
        }
        // Manejo de puntuación, excepto el guion bajo
        else if (ispunct(c) && c != '_' && c != '.')  
        {
            if (!palabra.empty())
            {
                textoSeparado.push(palabra);
                palabra = "";
            }
            textoSeparado.push(string(1, c));  // Se agrega el símbolo como un token
        }
        // Manejo de números
        else if (isdigit(c))  
        {
            palabra += c;  // Continuamos construyendo el número, ya sea entero o flotante
        }
        // Manejo de puntos decimales en un número flotante
        else if (c == '.')
        {
            // Si estamos en un número, seguimos construyéndolo con el punto
            if (!palabra.empty()) 
            {
                palabra += c;  // Agregamos el punto al número (ej. 2.2)
            }
            else
            {
                // Si el punto no está en medio de un número, lo tratamos como un símbolo
                if (!palabra.empty()) {
                    textoSeparado.push(palabra);
                    palabra = "";
                }
                textoSeparado.push(string(1, c));  // El punto como un token separado
            }
        }
        // Si es otro tipo de carácter, lo agregamos directamente a la palabra
        else
        {
            palabra += c;  
        }
    }

    // Si se quedó alguna palabra pendiente, la agregamos al final
    if (!palabra.empty())  
        textoSeparado.push(palabra);

    PrintQueue(textoSeparado);
    return textoSeparado;
}

void CheckNum(vector<pair<string, string>> tokens)
{
    cout<<"sientra"<<endl;;
    string d = "";
    for (const auto& par : tokens)
    {
        if(par.second == "D")
            d = par.first;
    }
    
    //revisar si siempre coincide.

    regex iType(R"((^\d+))");
    regex dType(R"(^(\d+(\.\d+)))");
    regex cType(R"(^('[a-zA-Z]'))");
    smatch match;

    cout<<"d vale: "<<d<<endl;

    for (const auto& par : tokens)
    {
        if(par.second == "NUM")
        {
            if(d == "int")
            {
                if(!regex_match(par.first, match, iType))
                {
                    cout<<"tipo de valor incorrecto"<<endl;
                    exit(1);
                }
            }
            if(d == "double")
            {
                if(!regex_match(par.first, match, dType) && !regex_match(par.first, match, iType))
                {
                    cout<<"tipo de valor incorrecto"<<endl;
                    exit(1);
                }

            }
            if(d == "char")
            {
                if(!regex_match(par.first, match, cType))
                {
                    cout<<"tipo de valor incorrecto"<<endl;
                    exit(1);
                }

            }
        }
    }
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

//Obtener el tipo de variable.
string GetVarType(unordered_map<string, pair<string, int>> variables, string var)
{
    for(const auto &pair : variables)
    {
        if(pair.first == var)
            return pair.second.first;
    }

    return "";
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

//Obtener el tipo de retorno de la funcion
string GetFunctionType(string var)
{
    for(const auto &pair : functions)
    {
        if(pair.first == var)
            return pair.second.first;
    }

    return "";
}

//Revisa si las condiciones son correctas
void ValidateCondition(const string& condition) 
{
    regex validConditionRegex(R"(^([a-zA-z_]\w*|\d+)\s*(==|!=|>=|>|<=|<|&&)\s*([a-zA-Z_]\w*|\d+))");
    smatch match;
    if (!regex_match(condition, match, validConditionRegex)) 
    {
        cout<<"Condicion invalida"<<endl;
        exit(1);
    }
}

void ValidateFor(const string& condition)
{
    regex validFor(R"(^([a-zA-Z_]\w*)\s*(=)\s*([a-zA-Z_]\w*|\d+)\s*(;)\s*([a-zA-z_]\w*|\d+)\s*(==|!=|>=|>|<=|<|&&)\s*([a-zA-Z_]\w*|\d+)\s*(;)\s*([a-zA-Z_]\w*)(\+\+|--)\s*)"); 
    smatch match;

    if(!regex_match(condition, match, validFor))
    {
        cout<<"Condicion invalida"<<endl;
        exit(1);
    }
}

//Separar el codigo en funciones. Guardar las variables globales al nodo root.
void CreateASTTree(Node &root, string &code) 
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
                bool declaration = false;
                tokens = SetTokens(match.str());
                CheckNum(tokens);
                for (const auto& par : tokens) 
                {
                    if(declaration && par.second == "D")
                    {
                        cout<<"Variable mal declara (doble tipo de dato)"<<endl;
                        exit(1);
                    }
                    if(par.second == "D" && !declaration)
                    {
                        tipo = par.first;
                        declaration = true;
                    }      
                    if (par.second == "N")
                    {
                        CheckReservedWords(par.first);
                        root.variables[par.first] = {tipo, root.variables[par.first].second + 1};
                    } 
                        
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
                auto functionNode = make_unique<Node>("function", "", root.variables, tokens[1].first);

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

                            // Agregar el parámetro al mapa de variables de la función
                            functionNode->variables[varName] = {varType, 1}; 

                            functionNode->tiposDeParametos.push_back(varType);
                            i++; // Avanzar al siguiente par de tokens (tipo y nombre)
                        }
                        else if (paramParsedTokens[i].second == "C")
                        {
                            // Ignorar comas, ya que solo separan los parámetros
                            continue;
                        }
                        else
                        {
                            cout << "Error en la definición de parámetros de la función." << endl;
                            exit(1);
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
                string functType = GetFunctionType(functionNode->nombreDeFuncion);
                smatch returnMatch;
                if (!regex_search(functionNode->body, returnMatch, regex(families["R"]))) {
                    if (functType != "void")                    
                    {
                        cout<<"Funcion "<<functionNode->nombreDeFuncion<<" distinta de void sin un return"<<endl;
                        cout<<"cuerpo de la funcion: "<<functionNode->body<<endl;
                        exit(1);
                    }
                }
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

//Revisar recurisvamente las funciones para encontrar bucles, condicionales y declaraciones de variables.
void CheckNode(Node &nodo, Node &root)
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
                auto ifNode = make_unique<Node>("if", "", nodo.variables, nodo.nombreDeFuncion);

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

                CheckNode(*ifNode, root);

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
                auto loopNode = make_unique<Node>("loop", "", nodo.variables, nodo.nombreDeFuncion);   

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

                CheckNode(*loopNode, root);

                nodo.AddNode(move(loopNode));

                code = code.substr(pos);
                matched = true;
                continue;
            }
        }        
        // Procesar declaraciones de variables locales
        if (!matched && regex_search(code, match, regex(families["V"])))
        {
            string asignationDataType = "";
            if (match.position() == 0)
            {
                string tipo = "";
                tokens = SetTokens(match.str());
                CheckNum(tokens);
                PrintTokens(tokens);
                string anterior = "";
                string actual = "";

                bool declaration = false;
                for (const auto& par : tokens) 
                {
                    if(declaration && par.second == "D")
                    {
                        cout<<"Variable mal declara (doble tipo de dato)"<<endl;
                        exit(1);
                    }
                    if(par.second == "D" && !declaration)
                    {
                        tipo = par.first;
                        declaration = true;
                    }
                        
                    actual = par.second;
                    //solo se deberia de guardar si estra atras de una coma o atras de un tipo de dato
                    if (par.second == "N" && (anterior == "C" || anterior == "D" )) 
                    {
                        nodo.variables[par.first] = nodo.variables.count(par.first) 
                                           ? make_pair(tipo, nodo.variables[par.first].second + 1) 
                                           : make_pair(tipo, 1);  
                    }

                    //int suma = a + b
                    if(par.second == "N")
                    {
                        CheckReservedWords(par.first);
                        CheckVariables(nodo.variables, par.first);
                        string dataType = GetVarType(nodo.variables, par.first);
                        //
                        if(dataType != asignationDataType && asignationDataType != "")
                        {
                            cout<<"Asignacion de datos incorrecta en "<<par.first << endl;
                            exit(1);
                        }
                        asignationDataType = dataType;
                    }
                        
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
                string asignationDataType = "";
                tokens = SetTokens(match.str());
                CheckNum(tokens);
                for (const auto& par : tokens) 
                {
                    if(par.second == "D")
                    {
                        cout<<"Error en la asignacion de valores (doble tipo de dato)"<<endl;
                        exit(1);
                    }

                    if (par.second == "N")
                    {
                        CheckReservedWords(par.first);
                        CheckVariables(nodo.variables, par.first);
                        //referenciar el primer tipo de valor
                        //asignationDataType = datatype jeje
                        //string = dataType y el datatype se busca en el map de variables
                        string dataType = GetVarType(nodo.variables, par.first);
                        //
                        if(dataType != asignationDataType && asignationDataType != "")
                        {
                            cout<<"Asignacion de datos incorrecta en "<<par.first << endl;
                            exit(1);
                        }
                        asignationDataType = dataType;
                    } 
                        
                }
                code = match.suffix().str();
                matched = true;
                continue;
            }
        }

        //procesar returns
        if (!matched && regex_search(code, match, regex(families["R"])))
        {
            if (match.position() == 0)
            {
                string returnVar = match[1];
                //get the var type
                string varType = GetVarType(nodo.variables, returnVar);
                //get the function type
                string funcType = GetFunctionType(nodo.nombreDeFuncion);
                //comparar si son iguales
                if(varType != funcType)
                {
                    cout<<"Valor del return "<<returnVar<<" es invalido"<<endl;
                    exit(1);
                }
                code = match.suffix().str();
                matched = true;
                continue;
            }
        }

        //procesar llamadas a funciones
        if (!matched && regex_search(code, match, regex(families["CALL"])))
        {
            if (match.position() == 0)
            {
                tokens = SetTokens(match.str());

                string functionName;
                string varType;
                bool isAssignment = false;

                if (tokens[0].second == "N") // tipo 1: Func(a, b);
                {
                    functionName = tokens[0].first;
                }
                else if (tokens[3].second == "N") // tipo 2: int var = Func(a, b);
                {
                    functionName = tokens[3].first;
                    varType = tokens[0].first;
                    isAssignment = true;
                }
                else
                {
                    cout << "Error en la llamada a la funcion." << endl;
                    exit(1);
                }

                

                Node *functionNode = nullptr;
                for (const auto &child : root.hijos)
                {
                    if (child->nombreDeFuncion == functionName)
                    {
                        functionNode = child.get();
                        break;
                    }
                }

                string funcType = GetFunctionType(functionName);
                if (isAssignment && funcType == "void")
                {
                    cout << "No se puede asignar el retorno de una funcion void." << endl;
                    exit(1);
                }

                if (!isAssignment && funcType != "void")
                {
                    cout << "Llamada a funcion con tipo de retorno sin asignacion: " << functionName << endl;
                    exit(1);
                }

                size_t startParams = match.str().find('(');
                size_t endParams = match.str().find(')');
                string parameters = match.str().substr(startParams + 1, endParams - startParams - 1);
                vector<pair<string, string>> paramParsedTokens = SetTokens(parameters);

                vector<string> localVarTypes;
                for (const auto &paramToken : paramParsedTokens)
                {
                    if (paramToken.second == "N")
                    {
                        CheckVariables(nodo.variables, paramToken.first); // Verificar si la variable existe
                        localVarTypes.push_back(GetVarType(nodo.variables, paramToken.first)); // Obtener el tipo
                    }
                }

                if (localVarTypes.size() != functionNode->tiposDeParametos.size())
                {
                    cout << "Cantidad incorrecta de parametros introducidos en la funcion: " << functionName << endl;
                    cout<<localVarTypes.size()<<" y "<<functionNode->tiposDeParametos.size()<<endl;
                    exit(1);
                }

                for (int i = 0; i < localVarTypes.size(); i++)
                {
                    if (localVarTypes[i] != (functionNode->tiposDeParametos)[i])
                    {
                        cout << "Tipo de dato incorrecto en el parametro " << i + 1 << " de la funcion '" << functionName << "'." << endl;
                        exit(1);
                    }
                }

                if (isAssignment && varType != funcType)
                {
                    cout << "Tipo de asginacion de función a variable incorrecta." << endl;
                    exit(1);
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
    return make_unique<Node>("root", "", unordered_map<string, pair<string, int>>(), "");
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
    CreateASTTree(*root, singleLineText);
    
    cout<<"Funciones"<<endl;
    PrintMap(functions);

    cout << "Verificando nodos hijos:" << endl;
    for (const auto& child : root->hijos) {
        CheckNode(*child, *root);
        PrintChildren(*child);
    }

    cout << "Analisis completado exitosamente." << endl;

    return 0;
}