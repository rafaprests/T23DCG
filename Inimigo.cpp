#include "Inimigo.h"

// Construtor
Inimigo::Inimigo(float rotacaoInicial, const Ponto& posicaoInicial)
    : Rotacao(rotacaoInicial), Posicao(posicaoInicial), r(1.0), g(0.0), b(0.0) {
    // Inicializa atributos conforme necessário
}

// Método para atualizar a posição do inimigo com base no tempo decorrido
void Inimigo::AtualizaPosicao(double tempoDecorrido, Ponto max, Ponto min) {
    // Implemente a lógica para atualizar a posição do inimigo aqui
    // Por exemplo, movimento linear com base na velocidade e direção
    // Atualize a posição do inimigo usando lógica de movimento conforme necessário
    // Certifique-se de verificar limites como max e min para evitar sair das fronteiras
}
