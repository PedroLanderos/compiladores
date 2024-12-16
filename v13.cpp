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



casos a probar:
1. declaracion de funciones void y llamarlas asignandolas a una variable (debe marcar error)
2. declaracion de funciones distintas a void y llamarlas sin asignar (debe marcar error)
3. declaracion de una funcion con n argumentos, pasandole x argumentos donde x != n (debe marcar error)
4. declaracion y asignacion de funciones con operaciones aritmeticas que incluyan variables y numeros
5. llamado a funciones pasandole argumentos de tipo numerico y de tipo variable
6. usar un return en una funcion de tipo void 
7. poner una funcion distinta de void sin un return
8. declarar una variable dentro de la llamada a una funcion (como argumento)

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
    {"D", R"(int|double|char|void)"},
    {"FUNC", R"(^([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\))"},
    {"RETURN", R"(return)"},                             // tipos de dato
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
    {"R", R"(\s*return\s+(([a-zA-Z_][a-zA-Z0-9_]*|\d+(\.\d+)?|'[a-zA-Z]'|\([^\)]*\))(\s*[\+\-\*\/]\s*([a-zA-Z_][a-zA-Z0-9_]*|\d+(\.\d+)?|'[a-zA-Z]'|\([^\)]*\)))*\s*)\s*;)"},
    {"CALL", R"(^\s*(\b\w+\s+\w+\s*=\s*)?\b\w+\s*\([^;]*\)\s*;)"},
    {"V", R"(^\s*(int|float|double|char)\s+([a-zA-Z_][a-zA-Z0-9_]*)(\s*=\s*[^,;]+(\([^)]*\))?[^,;]*)?(\s*,\s*[a-zA-Z_][a-zA-Z0-9_]*\s*(=\s*[^,;]+(\([^)]*\))?[^,;]*)?)*\s*;)"}
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

void PrintTokens(const vector<pair<string, string>>& tokens) 
{
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

bool IsFunctionName(const std::string& palabra) 
{
    if (palabra.empty()) return false;
    for (char c : palabra) {
        if (!std::isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

//Funcion para separar un string en palabras.
queue<string> SeparateText(const std::string& texto) 
{
    std::queue<std::string> textoSeparado;
    std::string palabra = "";
    bool enComillas = false;
    bool enFuncion = false;  // Para detectar que estamos dentro de una función
    int parenCount = 0;      // Contador de paréntesis para detectar si estamos dentro de los paréntesis de una función

    for (size_t i = 0; i < texto.size(); ++i) {
        char c = texto[i];

        // Manejo de comillas simples para los caracteres literales
        if (c == '\'' && !enComillas && !enFuncion) {
            enComillas = true;
            palabra += c;
        }
        else if (c == '\'' && enComillas && !enFuncion) {
            enComillas = false;
            palabra += c;
            textoSeparado.push(palabra);
            palabra = "";
        }
        // Manejo de paréntesis que pueden indicar el inicio de una función
        else if (c == '(' && !enComillas) {
            if (IsFunctionName(palabra)) {
                enFuncion = true;
                parenCount = 1;
                palabra += c;
            }
            else {
                if (!palabra.empty()) {
                    textoSeparado.push(palabra);
                    palabra = "";
                }
                textoSeparado.push(std::string(1, c));
            }
        }
        // Si estamos dentro de una función, seguimos acumulando hasta el paréntesis de cierre
        else if (enFuncion) {
            palabra += c;

            // Actualizamos el contador de paréntesis
            if (c == '(') {
                parenCount++;
            }
            else if (c == ')') {
                parenCount--;
            }

            // Si el contador de paréntesis llega a 0, terminamos la función
            if (parenCount == 0) {
                textoSeparado.push(palabra);
                palabra = "";
                enFuncion = false;
            }
        }
        // Manejo de espacios
        else if (std::isspace(c)) {
            if (!palabra.empty()) {
                textoSeparado.push(palabra);
                palabra = "";
            }
        }
        // Manejo de puntuación, excepto el guion bajo y el punto
        else if (std::ispunct(c) && c != '_' && c != '.') {
            if (!palabra.empty()) {
                textoSeparado.push(palabra);
                palabra = "";
            }
            textoSeparado.push(std::string(1, c));
        }
        // Manejo de números
        else if (std::isdigit(c)) {
            palabra += c;
        }
        // Manejo de puntos decimales en un número flotante
        else if (c == '.') {
            if (!palabra.empty() && std::isdigit(palabra.back())) {
                palabra += c;
            }
            else {
                if (!palabra.empty()) {
                    textoSeparado.push(palabra);
                    palabra = "";
                }
                textoSeparado.push(std::string(1, c));
            }
        }
        // Si es otro tipo de carácter, lo agregamos directamente a la palabra
        else {
            palabra += c;
        }
    }

    // Si quedó alguna palabra pendiente, la agregamos al final
    if (!palabra.empty()) {
        textoSeparado.push(palabra);
    }

    PrintQueue(textoSeparado);
    return textoSeparado;
}

void CheckNum(vector<pair<string, string>> tokens)
{
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

string SelectNumFamily(string num)
{
    string type = "";

    regex iType(R"((^\d+))");
    regex dType(R"(^(\d+(\.\d+)))");
    regex cType(R"(^('[a-zA-Z]'))");
    smatch match;

    if(regex_match(num, match, iType))
        type = "int";
    else if(regex_match(num, match, dType))
        type = "double";
    else if(regex_match(num, match, cType))
        type = "char";

    return type;
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

void FunctionExists(string name) {
    // Imprimir todo el mapa de funciones
    cout << "Funciones registradas:" << endl;
    for (const auto& entry : functions) {
        cout << "Funcion: " << entry.first  // Nombre de la función (clave)
             << ", Tipo: " << entry.second.first  // Tipo de la función (primer valor del pair)
             << ", Numero de parametros: " << entry.second.second  // Número de parámetros (segundo valor del pair)
             << endl;
    }

    // Buscar la función en el mapa
    auto it = functions.find(name);
    if (it == functions.end()) {
        cout << "La funcion " << name << " no ha sido declarada" << endl;
        exit(1);  // Termina el programa si no existe la función
    } else {
        cout << "La funcion " << name << " ha sido declarada" << endl;
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

void ValidateFunctionsParenthesis(const string& condition)
{
    regex validP(R"(^\s*(\s*[a-zA-Z_][a-zA-Z0-9_]*|\d+(\.\d+)?|'.')\s*(\s*,\s*(\s*[a-zA-Z_][a-zA-Z0-9_]*|\d+(\.\d+)?|'.')\s*)*$|^\s*$)");
    smatch match;
    
    if(!regex_match(condition, match, validP))
    {
        cout<<"parantesis invalido en la funcion"<<endl;
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
                string functionName = tokens[1].first; 
                size_t parenPos = functionName.find('(');
                functionName = functionName.substr(0, parenPos);
                AddGlobalFunctions(functionName, tokens[0].first);  
                cout<<"nombre de la funcion a agregar: "<<tokens[1].first<<endl;
                auto functionNode = make_unique<Node>("function", "", root.variables, functionName);

                //revisar parametros de la funcion
                string functionSignature = tokens[1].first;
                size_t startParams = tokens[1].first.find('(');
                size_t endParams = tokens[1].first.find(')');
                
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

void ValidateFunctions(string func, string tipo, Node &nodo, Node &root)
{
    size_t startParams2 = func.find('(');
    string funcName = func.substr(0, startParams2);
    string funcType = GetFunctionType(funcName);
    FunctionExists(funcName);

    if(funcType != tipo)
    {
        cout<<"La funcion: "<<funcName<<" no es del mismo tipo de la declaracion de variables"<<endl;
        exit(1);
    }   

    size_t startParams = func.find('(');
    size_t endParams = func.find(')');
    string parameters = func.substr(startParams + 1, endParams - startParams - 1);

    cout<<"cosa a validar: "<<parameters<<endl;
    ValidateFunctionsParenthesis(parameters);

    vector<pair<string, string>> tempTokens = SetTokens(parameters);
    vector<string> localVarTypes;

    for(const auto& var : tempTokens)
    {
        if(var.second == "N")
        {
            CheckVariables(nodo.variables, var.first);
            localVarTypes.push_back(GetVarType(nodo.variables, var.first));
        }
        if(var.second == "NUM")
        {
            string n = SelectNumFamily(var.first);
            localVarTypes.push_back(n);
        }
    }

    Node *functionNode = nullptr;

    for(const auto &child : root.hijos)
    {
        if(child->nombreDeFuncion == funcName)
        {
            functionNode = child.get();
            break;
        }
    }

    if (localVarTypes.size() != functionNode->tiposDeParametos.size())
    {
        cout << "Cantidad incorrecta de parametros introducidos en la funcion: " << funcName << endl;
        cout<<localVarTypes.size()<<" y "<<functionNode->tiposDeParametos.size()<<endl;
        exit(1);
    }

    for (int i = 0; i < localVarTypes.size(); i++)
    {
        if (localVarTypes[i] != (functionNode->tiposDeParametos)[i])
        {
            cout<<localVarTypes[i]<<endl;
            cout<<(functionNode->tiposDeParametos)[i];
            cout << "Tipo de dato incorrecto en el parametro " << i + 1 << " de la funcion '" << funcName << "'." << endl;
            exit(1);
        }
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
                tokens = SetTokens(match.str());
                string tipo = tokens[0].first;
                cout<<"tipo es: "<<tipo<<endl;
                CheckNum(tokens);
                PrintTokens(tokens);
                string anterior = "";
                string actual = "";

                bool declaration = false;

                for (const auto& par : tokens) 
                {
                    //primero encontrar todos los que sean funciones
                    if(par.second == "FUNC")
                    {
                        /*
                        size_t startParams2 = par.first.find('(');
                        string functionName = par.first.substr(0, startParams2);

                        // Ver si la función existe
                        FunctionExists(functionName);
                        //obtener el tipo de la funcion y comparar si es el mismo tipo de la declaracion.
                        string funcType = GetFunctionType(functionName);
                        if(funcType != tipo)
                        {
                            cout<<"La funcion: "<<functionName<<" no es del mismo tipo de la declaracion de variables"<<endl;
                            exit(1);
                        }   
                        //comprobar si los parentesis de la funcion son correctos en sintaxis
                        size_t startParams = par.first.find('(');
                        size_t endParams = par.first.find(')');
                        string parameters = par.first.substr(startParams + 1, endParams - startParams - 1);
                        ValidateFunctionsParenthesis(parameters);
                        //hacer un bucle recorriendo los parentesis de la funcion y cada que se encuentre a una variable revisar, si la variable ya fue declarada, obtener su tipo y luego comparar si es el mismo que el de la asignacion ya que la funcion es el mismo tipo que la asignacion
                        vector<pair<string, string>> tempTokens = SetTokens(parameters);
                        vector<string> localVarTypes;
                        for(const auto& var : tempTokens)
                        {
                            if(var.second == "N")
                            {
                                cout<<"si entra"<<endl;
                                //revisar si la variable existe
                                CheckVariables(nodo.variables, var.first);
                               //si es nombre obtener su tipo de dato y guardarlo en la queue
                               localVarTypes.push_back(GetVarType(nodo.variables, var.first));
                               cout<<GetVarType(nodo.variables, var.first)<<endl;
                            }
                            if(var.second == "NUM")
                            {
                                //si es numero convertirlo a tipo y guardarlo en la queue
                                string n = SelectNumFamily(var.first);
                                localVarTypes.push_back(n);
                            }
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
                                cout<<localVarTypes[i]<<endl;
                                cout<<(functionNode->tiposDeParametos)[i];
                                cout << "Tipo de dato incorrecto en el parametro " << i + 1 << " de la funcion '" << functionName << "'." << endl;
                                exit(1);
                            }
                        }
                        */
                        ValidateFunctions(par.first, tipo, nodo, root);

                    }
                }
                
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
                string tipo = GetVarType(nodo.variables, tokens[0].first); //obtener el tipo para usar la funcion de funciones jeje
                CheckNum(tokens);
                for (const auto& par : tokens) 
                {
                    if(par.second == "FUNC")
                    {
                        ValidateFunctions(par.first, tipo, nodo, root);
                    }

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
                //pueden ser return hola; o return 20; o return a + b + c + 20;
                
                //obtener el tipo de la funcion
                //pasar todo el return a tokens despues de la palabra return
                //si es una variable comprobar que sea el mismo tipo, si es un numero comprobar que pertenezca al mismo tipo de la funcion y si alguno deja de ser valido terminar el bucle con un bool isValid = false y ya 

                bool isValidReturn = true;
                string funcType = GetFunctionType(nodo.nombreDeFuncion);

                size_t startParams = match.str().find("return");
                size_t endParams = match.str().find(';');
                string parameters = match.str().substr(startParams + 6, endParams - startParams - 1);
                tokens = SetTokens(parameters);
                PrintTokens(tokens);


                for (const auto &paramToken : tokens)
                {
                    string aux = "vacio";
                    if (paramToken.second == "N")
                    {
                        CheckVariables(nodo.variables, paramToken.first);
                        aux = GetVarType(nodo.variables, paramToken.first);
                        if(aux != funcType)
                        {
                            cout<<"valor de aux: "<<aux<<endl;
                            cout<<"valor de func type: "<<funcType<<endl;
                            cout<<funcType<<endl;
                            cout<<"Tipo incorrecto en return con: " << paramToken.first<<endl;
                            exit(1);
                        }
                    }
                    if (paramToken.second == "NUM")
                    {
                        aux = SelectNumFamily(paramToken.first);
                        cout<<"numero y tipo: "<<paramToken.first<<", "<<aux<<endl;
                        if(aux != funcType)
                        {
                            cout<<"valor de aux: "<<aux<<endl;
                            cout<<funcType<<endl;
                            cout<<"Tipo incorrecto en return con: " << paramToken.first<<endl;
                            exit(1);
                        }
                    }
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
                string functionName = tokens[0].first; 
                size_t parenPos = functionName.find('(');
                functionName = functionName.substr(0, parenPos);
                //
                string varType;
                bool isAssignment = false;
                cout<<"El nombre es: "<<functionName<<endl;


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

                size_t startParams = tokens[0].first.find('(');
                size_t endParams = tokens[0].first.find(')');
                string parameters = tokens[0].first.substr(startParams + 1, endParams - startParams - 1);
                ValidateFunctionsParenthesis(parameters);
                cout<<"los param: "<<parameters<<endl;
                vector<pair<string, string>> paramParsedTokens = SetTokens(parameters);

                vector<string> localVarTypes;
                for (const auto &paramToken : paramParsedTokens)
                {
                    if (paramToken.second == "N")
                    {
                        cout<<"variable a revisar: "<<paramToken.first<<endl;
                        CheckVariables(nodo.variables, paramToken.first); // Verificar si la variable existe
                        localVarTypes.push_back(GetVarType(nodo.variables, paramToken.first)); // Obtener el tipo
                    }
                    if(paramToken.second == "NUM")
                    {
                        localVarTypes.push_back(SelectNumFamily(paramToken.first));
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
void PrintMap(const unordered_map<string, pair<string, int>>& map) 
{
    for (const auto& pair : map) {
        cout << "Hash: " << pair.first 
             << ", Tipo: " << pair.second.first 
             << ", Frecuencia: " << pair.second.second 
             << endl;
    }
}

//Imprimir la informacion de los nodos
void PrintChildren(const Node& root) 
{
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
unique_ptr<Node> InitiTree() 
{
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