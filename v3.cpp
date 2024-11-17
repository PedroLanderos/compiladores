#include <iostream>
#include <regex>
#include <queue>
#include <fstream>
#include <map>
#include <unordered_map>

using namespace std;

unordered_map<string, int> frecuenciaNombres;

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

//separa un string en palabras y regresa una queue en orden de aparicion.
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

//Valida el estatus de la operacion para saber el tipo de error que se presenta.
void StatusCheck(int ordenCorrecto)
{
    switch (ordenCorrecto)
    {
    case 1:
        cout << "Error en orden de variables" << endl;
        exit(1);
    case 2:
        cout << "Nombre de variable repetida" << endl;
        exit(2);
    default:
        break;
    }
}

//Verifica si hay alguna variable repetida
bool VariableFrecuency()
{
    for (const auto &pair : frecuenciaNombres)
        {
            if (pair.second > 1)
                return false;
        }
    return true;
}

void EstadoInicial(string estado1, string estado2)
{
    if(estado1 != "D" || estado2 != "N")
        StatusCheck(1);
}

void printVariableFrequencies(const unordered_map<string, int>& frecuencias) {
    cout << "Frecuencia de Variables:" << endl;
    for (const auto& pair : frecuencias) {
        cout << "Variable: " << pair.first << ", Frecuencia: " << pair.second << endl;
    }
}

void VariablesCheck(queue<string> families)
{
    if(families.size() <= 1) return StatusCheck(1);;

    if(!VariableFrecuency()) return StatusCheck(2);

    string familiaActual = families.front(); families.pop();
    string familiaSig = families.front(); families.pop();
    
    EstadoInicial(familiaActual, familiaSig);
    
    
    int nVariables = 1;

    while(families.size() > 1)
    {
        
        familiaActual = familiaSig;
        familiaSig = families.front(); families.pop();
        if(familiaSig == "N") nVariables++;

        if (familiaActual == "D" && familiaSig != "N")
            StatusCheck(1);
        if (familiaActual == "N" && familiaSig != "C" && familiaSig != "END" && familiaSig != "E")
            StatusCheck(1);
        if (familiaActual == "S" && familiaSig != "NUM")
            StatusCheck(1);
        if (familiaActual == "E" && familiaSig != "NUM")
            StatusCheck(1);
        if (familiaActual == "NUM" && familiaSig != "S" && familiaSig != "END" && familiaSig != "C")
            StatusCheck(1);
        if (familiaActual == "C" && (familiaSig != "N" || (nVariables != frecuenciaNombres.size())))
            StatusCheck(1);
        if (familiaActual == "END" && (familiaSig != "D")) 
            StatusCheck(1);

        //StatusCheck(value);
    }

    familiaActual = familiaSig;
    familiaSig = families.front();

    if(familiaSig != "END" || (familiaActual != "NUM" && familiaActual != "N"))
        StatusCheck(1);
}

void Identify(string text)
{
    bool declaracion = false;
    queue<string> textSeparated = SeparateText(text);
    queue<string> identifiers;
    printQueue(textSeparated);

    while (!textSeparated.empty())
    {
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
        if(identificador == front) StatusCheck(1);
        if(identificador == "D") declaracion = true;
        if(identificador == "END") declaracion = false;
        if(identificador == "N" && declaracion) frecuenciaNombres[front]++;

        identifiers.push(identificador);
    }
    VariablesCheck(identifiers);
    printQueue(identifiers);

}

int main() {
    fstream archivo("codigo.c");
    string linea, singleLineText;
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            singleLineText += linea + " ";
        }
        archivo.close();

        Identify(singleLineText);
        cout << endl;
    } else {
        cout << "Archivo faltante" << endl;
    }
}