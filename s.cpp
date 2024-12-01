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

void SeparateText()
{
    queue<string> textoSeparado;
    string palabra = "";
    bool enComillas = false; 
    string texto = "double p = 2.2;";
    char anterior = '\0';

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
        else if (ispunct(c) && c != '_')  
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
            cout<<anterior<<endl;
            // Si estamos en un número, seguimos construyéndolo con el punto
            if (!palabra.empty() && (anterior != '\0' && isdigit(anterior))) 
            {
                palabra += c;  // Agregamos el punto al número (ej. 2.2)
                cout<<"siii"<<endl;
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

        anterior = c;
    }

    // Si se quedó alguna palabra pendiente, la agregamos al final
    if (!palabra.empty())  
        textoSeparado.push(palabra);

    while(!textoSeparado.empty())
    {
        cout<<textoSeparado.front()<<endl;
        textoSeparado.pop();
    }
}

int main(int argc, char const *argv[])
{
    SeparateText();
    return 0;
}
