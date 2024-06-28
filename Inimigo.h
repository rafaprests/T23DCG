#ifndef Inimigo_hpp
#define Inimigo_hpp
#include "Ponto.h"

class Inimigo{
public:
    Inimigo(float rotacaoInicial, const Ponto& posicaoInicial);
    Ponto Posicao;
    float Rotacao;
    void AtualizaPosicao(double tempoDecorrido, Ponto max, Ponto min);
    double r,g,b;
    };


#endif /* Inimigo_hpp */
