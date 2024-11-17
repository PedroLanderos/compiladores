#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <regex>

using namespace std;    

vector<pair<string,string> >familias;
vector< vector<string> >patronesInt(4);

int main() {
    familias.push_back({"TD", "\\bint"});                              // Tipo de Dato
    familias.push_back({"ID", "(\\b[a-zA-Z_][a-zA-Z0-9]*)(?!\\w*\\|)"}); // Identificador
    familias.push_back({"SA", "[+*/-]"});                               // Simbolo Arietmetico
    familias.push_back({"IG", "[=]"});                                  // Igual
    familias.push_back({"NUM", "[0-9]+\\b"});                              // Numero
    familias.push_back({"SP", "[,]"});                                  // Separador (,)
    familias.push_back({"FN", "[;]"});



    ifstream archivo("ejer1.txt");  // Abrir el archivo en modo de lectura
    string linea;

    if (archivo.is_open()) {
        // Leer el archivo línea por línea
        while (getline(archivo, linea)) {
            string aux=linea;
            for(pair<string,string>a:familias){
                regex e(a.second);
                aux=regex_replace (aux,e,'|' + a.first + '|');
            }
        cout<<linea<<'\n'<<aux<<"\n\n";

        }
        archivo.close();  // Cerrar el archivo
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }

    return 0;
}
