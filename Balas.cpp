#include "Balas.h"

void BalasPonto(Ponto &p, Ponto &out)
{
    GLfloat ponto_novo[4];
    GLfloat matriz_gl[4][4];
    int  i;
    
    glGetFloatv(GL_MODELVIEW_MATRIX,&matriz_gl[0][0]);
    
    for(i=0;i<4;i++)
    {
        ponto_novo[i]= matriz_gl[0][i] * p.x+
        matriz_gl[1][i] * p.y+
        matriz_gl[2][i] * p.z+
        matriz_gl[3][i];
    }
    out.x=ponto_novo[0];
    out.y=ponto_novo[1];
    out.z=ponto_novo[2];
}

Ponto BalasPonto(Ponto P)
{
    Ponto temp;
    BalasPonto(P, temp);
    return temp;
}
Balas::Balas()
{
    cout << "Instanciado..." << endl;
    Rotacao = 0;
    Posicao = Ponto(0,0,0);
    Escala = Ponto(1,1,1);
}

void Balas::desenha()
{
    // Aplica as transformacoes geometricas no modelo
    glPushMatrix();
        glTranslatef(Posicao.x, Posicao.y, 0);
        glRotatef(Rotacao, 0, 0, 1);
        glScalef(Escala.x, Escala.y, Escala.z);
        
        // Obtem a posicao do ponto 0,0,0 no SRU
        // Nao eh usado aqui, mas eh util para detectar colisoes
        Ponto PosicaoDaBala;
        PosicaoDaBala = BalasPonto(Ponto (0,0,0));
        
        (*modelo)(); // desenha a instancia
        
    glPopMatrix();
}

void Balas::AtualizaPosicao(double tempoDecorrido)
{
    Posicao.x += 3 * Deslocamento.x;
    Posicao.y += 3 * Deslocamento.y;
}
