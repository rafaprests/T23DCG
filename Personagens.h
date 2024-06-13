#ifndef Personagens_hpp
#define Personagens_hpp
#include <iostream>
using namespace std;

#include "Poligono.h"
#include "Balas.h"
typedef void TipoFuncao();

class Personagens{
public:
    //Poligono *modelo;
    Personagens();
    TipoFuncao *modelo;
    Ponto Posicao, Escala, Deslocamento;
    Balas vetorDeBalas[10];
    int nInstanciasBalas = 0;
    float Largura, Altura;
    float Rotacao;
    void desenha();
    void AtualizaPosicao(double tempoDecorrido, Ponto max, Ponto min);
    double r,g,b;
    };


#endif /* Personagens_hpp */
