#include "Personagens.h"

void PersonagensPonto(Ponto &p, Ponto &out)
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

Ponto PersonagensPonto(Ponto P)
{
    Ponto temp;
    PersonagensPonto(P, temp);
    return temp;
}
Personagens::Personagens()
{
    cout << "Instanciado..." << endl;
    Rotacao = 0;
    Posicao = Ponto(0,0,0);
    Escala = Ponto(1,1,1);
}

void Personagens::desenha()
{
    // Aplica as transformacoes geometricas no modelo
    glPushMatrix();
        glTranslatef(Posicao.x, Posicao.y, 0);
        glRotatef(Rotacao, 0, 0, 1);
        glScalef(Escala.x, Escala.y, Escala.z);
        
        // Obtem a posicao do ponto 0,0,0 no SRU
        // Nao eh usado aqui, mas eh util para detectar colisoes
        Ponto PosicaoDoPersonagem;
        PosicaoDoPersonagem = PersonagensPonto(Ponto (0,0,0));
        
        (*modelo)(); // desenha a instancia
        
    glPopMatrix();
}

void Personagens::AtualizaPosicao(double tempoDecorrido, Ponto min, Ponto max)
{
    int direcao = rand() % 4;

    switch (direcao)
    {
        //cima
        case 0:
            Posicao.y += 1;
            if(Posicao.y >= max.y){
                Posicao.y -= 1;
            }
            break;
        
        //esquerda
        case 1:
            Posicao.x -= 1;
            if(Posicao.x <= min.y){
                Posicao.x += 1;
            }
            break;
        //baixo
        case 2:
            Posicao.y -= 1;
            if(Posicao.y <= min.y){
                Posicao.y += 1;
            }
            break;
        //direita
        case 3:
            Posicao.x += 1;
            if(Posicao.x >= max.x){
                Posicao.x -= 1;
            }
            break;
    }
}
