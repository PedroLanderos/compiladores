//version 2
//landeros cortes pedro jonas
//compilador de c

#include <iostream>
#include <regex>
#include <queue>
#include <fstream>
#include <map>
#include <unordered_map>

using namespace std;

/*
para declaracion de variables debe tener el sig orden:
    caso 1:
    int variable;

    caso2:
    int variable = 10;

    caso 3:
    int variable1, variable2, variable3;

    caso 4:
    int variable = 10, variable2 = 20, variable3 = 30;

    caso 5(variables con valores y sin valores combinadas):
    int variable = 10, variable2, variable3 = 30;

Reglas:

1. siempre se comienza con un tipo de variable
2. despues del tipo de variable siempre hay un nombre
3. Despues del tipo de familia nombre puede haber un simbolo "=" o un simbolo "; o un simbolo ","
4. despues del simbolo "=" siempre debe haber un numero y despues de un numero puede haber un ";", un ",", un simbolo "+ - * /"
5. al final siempre debe haber un ;
*/

unordered_map<string, int> frecuenciaNombres;

void ValidacionDeCodigo(int ordenCorrecto)
{
    switch (ordenCorrecto)
    {
    case 1:
        cout << "Error en orden de variables" << endl;
        exit(1);
    case 2:
        cout << "Nombre de variable repetida" << endl;
        exit(2);
    case 3:
        cout<<"elemento sin familia"<<endl;
        exit(3);
    default:
        break;
    }
}

bool VerificarNombres(int caso, string palabra)
{
    switch (caso)
    {
    case 1: // ver si no hay variables repetidas
        for (const auto &pair : frecuenciaNombres)
        {
            if (pair.second > 1)
                return false;
        }
        break;
    case 2: // ver si la variable ya estaba declarada
        if (frecuenciaNombres.find(palabra) == frecuenciaNombres.end())
            return false; 
        break;
    default:
        break;
    }
    return true;
}

int DeclaracionVariablesCorrecta(queue<string> families, queue<string> palabras)
{
    int initialSize = families.size();
    if (!VerificarNombres(1, "test"))
        return 2;

    if (families.empty() || palabras.empty())
        return 1;

    string familiaActual;
    string familiaSig;

    //si el map de los nombres esta vacio:
    if (frecuenciaNombres.empty())
    {
        familiaActual = families.front(); families.pop();
        familiaSig = families.front(); families.pop();
        if (familiaActual != "D" || familiaSig != "N")
            return 1;
        palabras.pop();
        palabras.pop();
    }
    
    while (families.size() > 1)
    {
        if(initialSize != families.size()) palabras.pop();
        familiaActual = familiaSig;
        familiaSig = families.front(); families.pop();
        

        if (familiaActual == "D" && familiaSig != "N")
            return 1;
        if (familiaActual == "N" && familiaSig != "C" && familiaSig != "END" && familiaSig != "E")
            return 1;
        if (familiaActual == "S" && familiaSig != "NUM")
            return 1;
        if (familiaActual == "E" && familiaSig != "NUM")
            return 1;
        if (familiaActual == "NUM" && familiaSig != "S" && familiaSig != "END" && familiaSig != "C")
            return 1;
        if (familiaActual == "C" && familiaSig != "N" && !VerificarNombres(2, palabras.front()))
            return 1;
        if (familiaActual == "END")
        {
            if (familiaSig != "D")
            {
                if (VerificarNombres(2, palabras.front()) == false) return 1; //true
            }
        }

    }
    
    familiaActual = familiaSig;
    familiaSig = families.front();
    if(familiaSig != "END")
        return 1;
    else
    {
        if (familiaActual != "NUM" && familiaActual != "N")
            return 1;
    }

    return 0; // si el codigo esta bien escrito
}

map<string, string> family = {
    {"D", R"(int|float|double|char)"},       // tipos de dato (data types)
    {"N", R"([a-zA-Z_][a-zA-Z0-9_]*)"},     // nombres (names)
    {"S", R"([\+\-\*/%])"},                 // simbolos (symbol)
    {"E", R"(=)"},                          // igual (equal)
    {"NUM", R"(\d+)"},                      // numeros (numbers)
    {"C", R"(,)"},                          // coma
    {"END", R"(;)"}                         // punto y coma o finalizacion de linea (end)
};

void printQueue(queue<string> queue)
{
    while (!queue.empty())
    {
        string front = queue.front(); queue.pop();
        cout << front << " ";
    }
    cout << endl;
}

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

// identifica a que familia pertenece y los va almacenando en una queue.
void Identify(string text)
{
    queue<string> textSeparated = SeparateText(text);
    queue<string> textToSend = textSeparated;
    queue<string> identifiers;
    printQueue(textSeparated);
    string prevFamily;

    do{
        string front = textSeparated.front(); textSeparated.pop();
        string identificador = front;

        for (const auto &pair : family)
        {
            regex reg(pair.second);
            if (regex_match(front, reg))
            {
                identificador = pair.first;
                break;
            }
        }

        if (identificador == "N" && prevFamily == "D")
            frecuenciaNombres[front]++;
        else if (identificador == front) ValidacionDeCodigo(3);

        prevFamily = identificador;
        identifiers.push(identificador);
    }while(textSeparated.size() > 0);
    
    int ordenCorrecto = DeclaracionVariablesCorrecta(identifiers, textToSend);

    ValidacionDeCodigo(ordenCorrecto);
    printQueue(identifiers);
}

int main()
{
    fstream archivo("codigo.c");
    string linea;

    if (archivo.is_open())
    {
        while (getline(archivo, linea))
        {
            string lineaActual = linea;
            Identify(lineaActual);
            cout << endl;
        }
        archivo.close();
    }
    else
    {
        cout << "Archivo faltante" << endl;
    }
}