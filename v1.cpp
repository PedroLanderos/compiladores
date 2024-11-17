#include <iostream>
#include <regex>
#include<queue>
#include <fstream>
#include<map>

using namespace std;

map<string, string> family = {
    {"D", R"(int|float|double|char)"}, //tipos de dato (data types)
    {"N", R"([a-zA-Z_][a-zA-Z0-9_]*)"}, //nombres (names)
    {"S", R"([\+\-\*/%])"}, //simbolos (symbol)
    {"E", R"(=)"}, //igual (equal)      
    {"NUM", R"(\d+)"}, //numberos (numbers)                    
    {"C", R"(,)"}, //coma           
    {"END", R"(;)"}, //punto y coma o finalizacion de linea (end)                            
};

void printQueue(queue<string> queue)
{
    while(!queue.empty())
    {
        string front = queue.front(); queue.pop();
        cout<<front<<" ";
    }
    cout<<endl;
}

queue<string> SeparateText(string texto)
{
    queue<string> textoSeparado;
    string palabra = "";

    for(char c : texto)
    {
        if(isspace(c))
        {
            if (palabra != "")
            {
                textoSeparado.push(palabra);
                palabra = ""; 
            }
        }
        else if (ispunct(c) && c != '_') //simbolos diferentes de _
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

void Identify(string text)
{
    queue<string> textSeparated = SeparateText(text);
    queue<string> identifiers;
    printQueue(textSeparated);

    while(!textSeparated.empty())
        {
            string front = textSeparated.front(); textSeparated.pop();
            string identificador = front;

            for(const auto& pair : family)
            {
                regex reg(pair.second);
                if(regex_match(front, reg))
                {
                    identificador = pair.first; 
                    break;
                }
            }
            identifiers.push(identificador);
        }
    printQueue(identifiers);
}

int main() {
    
    fstream archivo("c.txt"); 
    string linea;

    if (archivo.is_open()) {
        while (getline(archivo, linea)) 
        {
            string lineaActual = linea;
            Identify(lineaActual);
            cout<<endl;
        }
        archivo.close();  
    } else {
        cout << "Archivo faltante" << endl;
    }
}