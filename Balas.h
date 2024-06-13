#ifndef Balas_hpp
#define Balas_hpp
#include <iostream>
using namespace std;

#include "Poligono.h"
typedef void TipoFuncao();

class Balas{
public:
    //Poligono *modelo;
    Balas();
    TipoFuncao *modelo;
    Ponto Posicao, Escala, Deslocamento;
    float Largura, Altura;
    float Rotacao;
    void desenha();
    void AtualizaPosicao(double tempoDecorrido);
    double r,g,b;
    };


#endif /* Balas_hpp */
