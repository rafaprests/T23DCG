#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Temporizador.h"
#include "ListaDeCoresRGB.h"
#include "Ponto.h"
#include "Inimigo.h"

// Define constants for map elements
#define CORRIDOR 0
#define WALL 1
#define WINDOW 2
#define DOOR 3
#define PLAYER_START 4
#define GAS 5
#define ENEMY 6
#define CHAIR 7
#define TABLE 8

// variaveis relativas ao mapa
std::vector<std::vector<int>> mapa;
std::vector<std::vector<bool>> rotacaoMapa;

// variaveis relativar ao jogador
Ponto jogador;
bool andando = false;
double jogadorRotacao = 0.0; // Ângulo de rotação do jogador em graus
double jogadorVetorRotacao = 0.0;
double jogadorVelocidade = 0.5; // Velocidade de movimento do jogador
int energia = 100;
// 0: Primeira pessoa, 1: Visão de cima, 2: Terceira pessoa
int ModoDeVisao = 0;

// variaveis relativas aos combustiveis
int valor_de_reabastecimento = 20;
int nroCombustiveis = 10;

// variaveis relativas aos inimigos
std::vector<Inimigo> inimigos;
double inimigoVelocidade = 1; // Velocidade de movimento do inimigo
int nroInimigos = 10;

// variaveis relativas as telas
bool telaInicial = true;
bool telaVitoria = false;
bool telaFinal = false;
GLfloat AspectRatio, angulo = 0;
double nFrames = 0;
double TempoTotal = 0;
Ponto OBS;
Ponto ALVO;
Ponto VetorAlvo;

// variaveis relativas ao tempo
Temporizador T;
double AccumDeltaT = 0;

// Função para ler o mapa do arquivo
void LeMapa(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo do mapa!" << endl;
        exit(1);
    }

    std::string line;
    int linhaIndex = 0;

    while (getline(file, line))
    {
        std::vector<int> row;
        std::vector<bool> rotRow;

        std::stringstream ss(line);
        int value;
        int colunaIndex = 0;

        while (ss >> value)
        {
            row.push_back(value);

            // Verifica se precisa rotacionar
            bool rotate = false;
            if (value == WALL || value == WINDOW || value == DOOR)
            {
                // Verifica paredes, janelas ou portas adjacentes
                if (row.size() > 1 && row[row.size() - 2] == WALL)
                    rotate = true;
                else if (!rotRow.empty() && rotRow.back())
                    rotate = true;
            }
            else if (value == CHAIR)
            {
                // Verifica se há uma mesa na célula diretamente acima
                if (linhaIndex > 0 && mapa[linhaIndex - 1][colunaIndex] == TABLE)
                {
                    rotate = true;
                }
            }

            rotRow.push_back(rotate);

            if (ss.peek() == ',')
                ss.ignore();

            colunaIndex++;
        }

        mapa.push_back(row);
        rotacaoMapa.push_back(rotRow);
        linhaIndex++;
    }

    file.close();

    // Exibe o mapa lido para verificação
    for (size_t i = 0; i < mapa.size(); ++i)
    {
        for (size_t j = 0; j < mapa[i].size(); ++j)
        {
            cout << mapa[i][j] << " ";
        }
        cout << endl;
    }
}

void DesenhaParede(float x, float y, float z, bool rotate)
{
    float alturaParede = 2.7;
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y, z);
    glTranslatef(0, alturaParede / 2, 0);
    if (rotate)
        glRotatef(90, 0, 1, 0);

    glScalef(1, alturaParede, 0.25);
    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1, 1, 1);
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaPiso(float x, float y, float z)
{
    glPushMatrix();
    defineCor(DarkPurple);
    glTranslatef(x, y, z);
    glScalef(1, 0, 1);
    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1, 1, 1);
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaCombustivel(float x, float y, float z)
{
    float altura = 0.75;
    glPushMatrix();
    defineCor(OrangeRed);
    glTranslatef(x, y, z);

    glTranslatef(0, altura / 2, 0);
    glScalef(altura, altura, altura);

    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1, 1, 1);
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaJanela(float x, float y, float z, bool rotate)
{
    float alturaParede = 2.7;
    float alturaJanela = 1.1;
    float alturaInferiorJanela = 0.9;
    float espessuraParede = 0.25;
    float larguraJanela = 1.0;

    // Desenhar a parede inferior
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y + alturaInferiorJanela / 2, z);

    // Verifica se deve rotacionar a parede
    if (rotate)
        glRotatef(90, 0, 1, 0);

    glScalef(1, alturaInferiorJanela, espessuraParede);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();

    // Desenhar a janela
    glPushMatrix();
    defineCor(LightBlue);
    glTranslatef(x, y + alturaInferiorJanela + alturaJanela / 2, z);

    // Verifica se deve rotacionar a parede
    if (rotate)
        glRotatef(90, 0, 1, 0);

    glScalef(larguraJanela, alturaJanela, 0.1);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();

    // Desenhar a parede superior
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y + alturaInferiorJanela + alturaJanela + (alturaParede - (alturaInferiorJanela + alturaJanela)) / 2, z);

    // Verifica se deve rotacionar a parede
    if (rotate)
        glRotatef(90, 0, 1, 0);

    glScalef(1, alturaParede - (alturaInferiorJanela + alturaJanela), espessuraParede);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();
}

void DesenhaPorta(float x, float y, float z, bool rotate)
{
    float alturaParede = 2.7;
    float alturaPorta = 2.1;
    float espessuraParede = 0.25;
    float larguraPorta = 1;

    // Desenhar a porta (do chão até 2.1 metros)
    glPushMatrix();
    defineCor(Bronze);
    glTranslatef(x, y + alturaPorta / 2, z);
    // Verifica se deve rotacionar a parede
    if (rotate)
        glRotatef(90, 0, 1, 0);

    glScalef(larguraPorta, alturaPorta, 0.1);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();

    // Desenhar a parede superior (de 2.1 metros até 2.7 metros)
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y + alturaPorta + (alturaParede - alturaPorta) / 2, z);
    // Verifica se deve rotacionar a parede
    if (rotate)
        glRotatef(90, 0, 1, 0);

    glScalef(1, alturaParede - alturaPorta, espessuraParede);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();
}

void DesenhaInimigos()
{
    for (const auto &inimigo : inimigos)
    {
        glPushMatrix();
        glTranslatef(inimigo.Posicao.x, inimigo.Posicao.y, inimigo.Posicao.z);
        glRotatef(inimigo.Rotacao + 90.0f, 0.0f, 1.0f, 0.0f);

        // Corpo do Enderman
        glColor3f(0.0f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0.0, 0.0, 0.0);
        glScalef(0.2, 4.8, 0.2);
        glutSolidCube(1);
        glPopMatrix();

        // Cabeça do Enderman
        glColor3f(0.0f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0, 2.4, 0);
        glScalef(0.5, 0.5, 0.5);
        glutSolidCube(1);
        glPopMatrix();

        // Olhos do Enderman (branco)
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(-0.1, 2.4, 0.3);
        glutSolidCube(0.1);
        glTranslatef(0.2, 0, 0);
        glutSolidCube(0.1);
        glPopMatrix();

        // Braços do Enderman
        glColor3f(0.0f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0.2, 1.5, 0.0);
        glScalef(0.1, 1.0, 0.1);
        glutSolidCube(1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-0.2, 1.5, 0.0);
        glScalef(0.1, 1.0, 0.1);
        glutSolidCube(1);
        glPopMatrix();

        glPopMatrix();
    }
}

void DesenhaCadeira(float x, float y, float z, bool rotate)
{
    glPushMatrix();
    glTranslatef(x, y, z);

    if (rotate)
    {
        glRotatef(180, 0, 1, 0);
    }

    float alturaAssento = 0.6;
    float espessuraAssento = 0.05;
    float larguraAssento = 0.6;
    float profundidadeAssento = 0.6;
    float alturaEncosto = 0.6;
    float larguraEncosto = 0.6;
    float espessuraEncosto = 0.05;
    float alturaPerna = 0.6;
    float espessuraPerna = 0.05;

    // Assento
    defineCor(Brown);
    glPushMatrix();
    glTranslatef(0, alturaPerna + espessuraAssento / 2, 0);
    glScalef(larguraAssento, espessuraAssento, profundidadeAssento);
    glutSolidCube(1);
    glPopMatrix();

    // Encosto
    glPushMatrix();
    glTranslatef(0, alturaPerna + espessuraAssento + alturaEncosto / 2, -profundidadeAssento / 2 + espessuraEncosto / 2);
    glScalef(larguraEncosto, alturaEncosto, espessuraEncosto);
    glutSolidCube(1);
    glPopMatrix();

    // Pernas
    glPushMatrix();
    glTranslatef(larguraAssento / 2 - espessuraPerna / 2, alturaPerna / 2, profundidadeAssento / 2 - espessuraPerna / 2);
    glScalef(espessuraPerna, alturaPerna, espessuraPerna);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-larguraAssento / 2 + espessuraPerna / 2, alturaPerna / 2, profundidadeAssento / 2 - espessuraPerna / 2);
    glScalef(espessuraPerna, alturaPerna, espessuraPerna);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(larguraAssento / 2 - espessuraPerna / 2, alturaPerna / 2, -profundidadeAssento / 2 + espessuraPerna / 2);
    glScalef(espessuraPerna, alturaPerna, espessuraPerna);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-larguraAssento / 2 + espessuraPerna / 2, alturaPerna / 2, -profundidadeAssento / 2 + espessuraPerna / 2);
    glScalef(espessuraPerna, alturaPerna, espessuraPerna);
    glutSolidCube(1);
    glPopMatrix();

    glPopMatrix();
}

void DesenhaMesa(float x, float y, float z)
{
    glPushMatrix();
    glTranslatef(x, y, z);

    // Tampo da mesa
    defineCor(Brown);
    glPushMatrix();
    glTranslatef(0, 0.8, 0);
    glScalef(1, 0.05, 1);
    glutSolidCube(1);
    glPopMatrix();

    // Pernas da mesa
    glPushMatrix();
    glTranslatef(0.35, 0, 0.35);
    glScalef(0.05, 1.6, 0.05);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.35, 0, 0.35);
    glScalef(0.05, 1.6, 0.05);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.35, 0, -0.35);
    glScalef(0.05, 1.6, 0.05);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.35, 0, -0.35);
    glScalef(0.05, 1.6, 0.05);
    glutSolidCube(1);
    glPopMatrix();

    glPopMatrix();
}

void DesenhaLabirinto()
{
    for (size_t i = 0; i < mapa.size(); ++i)
    {
        for (size_t j = 0; j < mapa[i].size(); ++j)
        {
            float x = j * 1.0;
            float z = i * 1.0;

            switch (mapa[i][j])
            {
            case CORRIDOR:
                DesenhaPiso(x, 0, z);
                break;
            case WALL:
                DesenhaPiso(x, 0, z);
                if (rotacaoMapa[i][j])
                    DesenhaParede(x, 0, z, false);
                else
                    DesenhaParede(x, 0, z, true);
                break;
            case WINDOW:
                DesenhaPiso(x, 0, z);
                if (rotacaoMapa[i][j])
                    DesenhaJanela(x, 0, z, false);
                else
                    DesenhaJanela(x, 0, z, true);
                break;
            case DOOR:
                DesenhaPiso(x, 0, z);
                if (rotacaoMapa[i][j])
                    DesenhaPorta(x, 0, z, false);
                else
                    DesenhaPorta(x, 0, z, true);
                break;
            case PLAYER_START:
                mapa[i][j] = CORRIDOR;
                DesenhaPiso(x, 0, z);
                jogador.x = x;
                jogador.y = 0;
                jogador.z = z;
                break;
            case GAS:
                DesenhaPiso(x, 0, z);
                DesenhaCombustivel(x, 0, z);
                break;
            case CHAIR:
                DesenhaPiso(x, 0, z);
                if (rotacaoMapa[i][j])
                {
                    DesenhaCadeira(x, 0, z, true);
                }
                else
                {
                    DesenhaCadeira(x, 0, z, false);
                }
                break;
            case TABLE:
                DesenhaPiso(x, 0, z);
                DesenhaMesa(x, 0, z);
                break;
            }
        }
    }
}

void Setup2DProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-50.0, 50.0, -50.0, 50.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void DesenhaTelaInicial()
{
    glClear(GL_COLOR_BUFFER_BIT);

    defineCor(DarkPurple);
    glRasterPos2f(-15, 20);
    string texto = "Press SPACE to start";
    for (const char &c : texto)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
}

void DesenhaTelaFinal()
{
    glClear(GL_COLOR_BUFFER_BIT);

    defineCor(DarkPurple);
    glRasterPos2f(-10, 20);
    string texto1 = "GAME OVER";
    for (const char &c : texto1)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }

    glRasterPos2f(-8, 0);
    string texto2 = "Press R to restart";
    for (const char &c : texto2)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

void DesenhaTelaVitoria()
{
    glClear(GL_COLOR_BUFFER_BIT);

    defineCor(DarkPurple);
    glRasterPos2f(-20, 20);
    string texto = "You WON!";
    for (const char &c : texto)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
}

// Função para inicializar as posições dos combustiveis
void InicializaPosicoesCombustiveis(int quantidade)
{
    srand(time(nullptr)); // Inicializa a semente do gerador de números aleatórios

    for (int i = 0; i < quantidade; ++i)
    {
        int x, z;
        do
        {
            x = rand() % mapa[0].size();
            z = rand() % mapa.size();
        } while (mapa[z][x] != CORRIDOR);

        mapa[z][x] = GAS;
    }
}

// Função para inicializar as posições dos inimigos
void InicializaPosicoesInimigos(int quantidade)
{
    srand(time(nullptr)); // Inicializa a semente do gerador de números aleatórios

    for (int i = 0; i < quantidade; ++i)
    {
        int x, z;
        do
        {
            x = rand() % mapa[0].size();
            z = rand() % mapa.size();
        } while (mapa[z][x] != CORRIDOR);

        inimigos.emplace_back(0.0f, Ponto(x, 0.0f, z));
    }
}

void Restart()
{
    mapa.clear();
    rotacaoMapa.clear();
    andando = false;
    jogadorRotacao = 0.0;
    jogadorVetorRotacao = 0.0;
    energia = 100;
    // 0: Primeira pessoa, 1: Visão de cima
    ModoDeVisao = 0;

    // variaveis relativas aos inimigos
    inimigos.clear();

    // variaveis relativas as telas
    telaInicial = true;
    telaVitoria = false;
    telaFinal = false;
    GLfloat AspectRatio, angulo = 0;
    nFrames = 0;
    TempoTotal = 0;

    AccumDeltaT = 0;

    // chamada de alguns metodos novamente
    LeMapa("mapaOficial.txt");

    InicializaPosicoesCombustiveis(nroCombustiveis);
    InicializaPosicoesInimigos(nroInimigos);
}

void ColidiuComGas()
{
    // Encontra uma posição aleatória válida para o combustível
    int novoX, novoZ;
    do
    {
        novoX = rand() % mapa[0].size();
        novoZ = rand() % mapa.size();
    } while (mapa[novoZ][novoX] != CORRIDOR);
    // Move o combustível para a nova posição aleatória
    mapa[novoZ][novoX] = GAS;

    // Aumenta a energia do jogador
    if (energia < 100)
    {
        energia += valor_de_reabastecimento;
    }
}

void ColidiuComInimigo(int inimigoIndex)
{
    // Encontra uma posição aleatória válida para o inimigo
    int novoX, novoZ;
    do
    {
        novoX = rand() % mapa[0].size();
        novoZ = rand() % mapa.size();
    } while (mapa[novoZ][novoX] != CORRIDOR);

    // Atualiza a posição do inimigo no vetor
    inimigos[inimigoIndex].Posicao.x = novoX;
    inimigos[inimigoIndex].Posicao.z = novoZ;

    if (energia > 0)
    {
        energia -= 5;
    }
    else
    {
        telaFinal = true;
    }
}

void AtualizaPosicaoJogador()
{
    // Calcula a nova posição do jogador baseado na sua rotação e velocidade
    double rad = jogadorRotacao * M_PI / 180.0;
    double novoX = jogador.x + cos(rad) * jogadorVelocidade;
    double novoZ = jogador.z + sin(rad) * jogadorVelocidade;

    if (andando)
    {
        // Verifica se a nova posição é válida (não colide com paredes)
        int mapaX = static_cast<int>(novoX + 0.5);
        int mapaZ = static_cast<int>(novoZ + 0.5);

        if (mapa[mapaZ][mapaX] == CORRIDOR || mapa[mapaZ][mapaX] == GAS)
        {
            if (mapa[mapaZ][mapaX] == GAS)
            {
                ColidiuComGas();
            }

            // Remove o item colidido do mapa, se for GAS
            if (mapa[mapaZ][mapaX] == GAS)
            {
                mapa[mapaZ][mapaX] = CORRIDOR;
            }
            jogador.x = novoX;
            jogador.z = novoZ;
        }
    }
    // Atualizar o vetor alvo de acordo com a nova direção do jogador
    VetorAlvo.x = cos(rad);
    VetorAlvo.z = sin(rad);
    VetorAlvo.y = 0;
}

void ApontaInimigosJogador(Inimigo &i)
{
    Ponto pontoInimigo = i.Posicao;

    // Calcula o vetor entre o inimigo e o jogador
    float vetorX = jogador.x - pontoInimigo.x;
    float vetorZ = jogador.z - pontoInimigo.z;

    // Calcula o ângulo entre os vetores usando atan2, que lida corretamente com todos os quadrantes
    float anguloRad = atan2(vetorZ, vetorX);
    float anguloGraus = anguloRad * 180.0 / M_PI;

    // Ajusta a rotação do inimigo para apontar para o jogador
    i.Rotacao = -anguloGraus;
    i.VetorRotacao = anguloGraus;
}

void MoveInimigos()
{
    for (size_t i = 0; i < inimigos.size(); ++i)
    {
        auto &inimigo = inimigos[i];

        ApontaInimigosJogador(inimigo);

        // Calcula a nova posicao do inimigo baseado na sua rotacao e velocidade
        double rad = (inimigo.VetorRotacao) * M_PI / 180.0;
        double novoX = inimigo.Posicao.x + cos(rad) * inimigoVelocidade;
        double novoZ = inimigo.Posicao.z + sin(rad) * inimigoVelocidade;

        int mapaX = static_cast<int>(novoX + 0.5);
        int mapaZ = static_cast<int>(novoZ + 0.5);

        if (static_cast<int>(jogador.x + 0.5) == mapaX && static_cast<int>(jogador.z + 0.5) == mapaZ)
        {
            // Se colidiu com o jogador, chama a função de colisão
            ColidiuComInimigo(i);
            break;
        }
        else if (mapa[mapaZ][mapaX] == CORRIDOR)
        {
            // Atualizar a posição do inimigo se não houver colisão com obstáculos
            inimigo.Posicao.x = novoX;
            inimigo.Posicao.z = novoZ;
        }
        else
        {
            // Tentar mover em uma direção alternativa com pequenos incrementos de ângulo
            bool moved = false;
            for (int angleOffset = 10; angleOffset <= 360; angleOffset += 10)
            {
                double newRad = (inimigo.VetorRotacao + angleOffset) * M_PI / 180.0;
                double altX = inimigo.Posicao.x + cos(newRad) * inimigoVelocidade;
                double altZ = inimigo.Posicao.z + sin(newRad) * inimigoVelocidade;

                int altMapaX = static_cast<int>(altX + 0.5);
                int altMapaZ = static_cast<int>(altZ + 0.5);

                if (mapa[altMapaZ][altMapaX] == CORRIDOR)
                {
                    inimigo.Posicao.x = altX;
                    inimigo.Posicao.z = altZ;
                    inimigo.VetorRotacao += angleOffset;
                    moved = true;
                    break;
                }

                // Tentar direção oposta
                newRad = (inimigo.VetorRotacao - angleOffset) * M_PI / 180.0;
                altX = inimigo.Posicao.x + cos(newRad) * inimigoVelocidade;
                altZ = inimigo.Posicao.z + sin(newRad) * inimigoVelocidade;

                altMapaX = static_cast<int>(altX + 0.5);
                altMapaZ = static_cast<int>(altZ + 0.5);

                if (mapa[altMapaZ][altMapaX] == CORRIDOR)
                {
                    inimigo.Posicao.x = altX;
                    inimigo.Posicao.z = altZ;
                    inimigo.VetorRotacao -= angleOffset;
                    moved = true;
                    break;
                }
            }

            if (!moved)
            {
                // Se não conseguiu se mover em nenhuma direção, tentar uma direção oposta
                double reverseRad = (inimigo.VetorRotacao + 180) * M_PI / 180.0;
                double revX = inimigo.Posicao.x + cos(reverseRad) * inimigoVelocidade;
                double revZ = inimigo.Posicao.z + sin(reverseRad) * inimigoVelocidade;

                int revMapaX = static_cast<int>(revX + 0.5);
                int revMapaZ = static_cast<int>(revZ + 0.5);

                if (mapa[revMapaZ][revMapaX] == CORRIDOR)
                {
                    inimigo.Posicao.x = revX;
                    inimigo.Posicao.z = revZ;
                    inimigo.VetorRotacao += 180;
                }
            }
        }
    }
}

void DesenhaJogador()
{
    glPushMatrix();
    glTranslatef(jogador.x, jogador.y, jogador.z);
    glRotatef(jogadorVetorRotacao + 90.0, 0.0, 1.0, 0.0);

    // Cabeça do Steve (pele)
    glColor3f(0.87f, 0.72f, 0.53f);
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Corpo do Steve (camiseta azul)
    glColor3f(0.14f, 0.42f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    glScalef(0.5f, 0.7f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Braços do Steve (camiseta azul e pele)
    // Braço esquerdo
    glColor3f(0.14f, 0.42f, 0.8f);
    glPushMatrix();
    glTranslatef(-0.45f, 0.8f, 0.0f);
    glScalef(0.15f, 0.7f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Braço direito
    glColor3f(0.14f, 0.42f, 0.8f);
    glPushMatrix();
    glTranslatef(0.45f, 0.8f, 0.0f);
    glScalef(0.15f, 0.7f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Pernas do Steve (calça azul escuro)
    glColor3f(0.0f, 0.0f, 0.5f);
    // Perna esquerda
    glPushMatrix();
    glTranslatef(-0.15f, 0.3f, 0.0f);
    glScalef(0.2f, 0.7f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Perna direita
    glPushMatrix();
    glTranslatef(0.15f, 0.3f, 0.0f);
    glScalef(0.2f, 0.7f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void init(void)
{
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // Fundo de tela preto

    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade
    glClearDepth(1.0);       // Limpa o valor do Z-buffer para o valor máximo
    glDepthFunc(GL_LESS);    // Função de teste de profundidade
    glEnable(GL_CULL_FACE);  // Habilita o culling de faces
    glEnable(GL_NORMALIZE);  // Normaliza normais para cálculos corretos de iluminação
    glShadeModel(GL_FLAT);   // Modelo de sombreamento flat para um efeito mais simples

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    OBS = Ponto(0, 5, 10);
    ALVO = Ponto(0, 0, 0);
    VetorAlvo = ALVO - OBS;
}

void animate()
{
    double dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0 / 30)
    {
        AccumDeltaT = 0;
        angulo += 1;
        AtualizaPosicaoJogador();
        MoveInimigos();
        glutPostRedisplay();
    }
}

void DefineLuz(void)
{
    // Calcule a intensidade da luz com base na energia do jogador
    float intensidade = energia / 100.0f; // energia varia de 0 a 100

    // Defina as cores da luz, variando do vermelho ao branco
    GLfloat LuzAmbiente[] = {0.4f * intensidade, 0.4f * intensidade, 0.4f * intensidade};
    GLfloat LuzDifusa[] = {intensidade, intensidade * 0.3f, intensidade * 0.3f}; // mais vermelho conforme diminui energia
    GLfloat LuzEspecular[] = {0.9f * intensidade, 0.9f * intensidade, 0.9f * intensidade};
    GLfloat PosicaoLuz0[] = {0.0f, 3.0f, 5.0f}; // Posição da Luz

    GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
    glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT, GL_SPECULAR, Especularidade);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
}

void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar)
{
    // https://stackoverflow.com/questions/2417697/gluperspective-was-removed-in-opengl-3-1-any-replacements/2417756#2417756
    //  The following code is a fancy bit of math that is equivilant to calling:
    //  gluPerspective( fieldOfView/2.0f, width/height , 0.1f, 255.0f )
    //  We do it this way simply to avoid requiring glu.h
    // GLfloat zNear = 0.1f;
    // GLfloat zFar = 255.0f;
    // GLfloat aspect = float(width)/float(height);
    GLfloat fH = tan(float(fieldOfView / 360.0f * 3.14159f)) * zNear;
    GLfloat fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void PosicUser()
{
    // Define os parâmetros da projeção Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    MygluPerspective(60, AspectRatio, 0.1, 50); // Projecao perspectiva

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (ModoDeVisao == 0)
    {                                                                                              // Primeira pessoa
        gluLookAt(jogador.x, jogador.y + 1.5, jogador.z,                                           // Posiçao do observador
                  jogador.x + VetorAlvo.x, jogador.y + 1.5 + VetorAlvo.y, jogador.z + VetorAlvo.z, // Posiçao do alvo
                  0.0, 1.0, 0.0);
    }
    else if (ModoDeVisao == 2)
    {
        gluLookAt(50, 50, 10,
                  50, 0, 10,
                  0.0, 0.0, -1.0);
    }
    else if (ModoDeVisao == 1)
    { // Terceira pessoa (visão de trás)
        // Definir a posição da câmera atrás do jogador
        float distanciaAtras = 6.0f; // Distância atrás do jogador
        float alturaCamera = 5.0f;   // Altura da câmera

        // Calcula a posição da câmera atrás do jogador
        float cameraX = jogador.x - VetorAlvo.x * distanciaAtras;
        float cameraY = jogador.y + alturaCamera;
        float cameraZ = jogador.z - VetorAlvo.z * distanciaAtras;

        gluLookAt(cameraX, cameraY, cameraZ,             // Posiçao do observador
                  jogador.x, jogador.y + 1.5, jogador.z, // Posiçao do alvo (cabeça do jogador)
                  0.0, 1.0, 0.0);                        // Up vector
    }
}

void reshape(int w, int h)
{
    // Evita divisão por zero, no caso de uma janela com largura 0.
    if (h == 0)
        h = 1;

    AspectRatio = 1.0f * w / h;

    // Seta a viewport para ocupar toda a janela
    glViewport(0, 0, w, h);

    // Define os parâmetros da projeção e ajusta a relação de aspecto
    PosicUser();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    if (telaInicial)
    {
        Setup2DProjection();
        DesenhaTelaInicial();
    }
    else
    {
        if (telaVitoria)
        {
            Setup2DProjection();
            DesenhaTelaVitoria();
        }
        else
        {
            if (telaFinal)
            {
                Setup2DProjection();
                DesenhaTelaFinal();
            }
            else
            {
                DefineLuz();
                PosicUser();
                DesenhaLabirinto();
                DesenhaJogador();
                DesenhaInimigos();
            }
        }
    }
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:     // Termina o programa qdo
        exit(0); // a tecla ESC for pressionada
        break;
    case ' ':
        if (telaInicial)
        {
            telaInicial = false;
        }
        else
        {
            andando = !andando;
        }
        break;
    case 'r':
        if (telaFinal)
        {
            Restart();
        }
        break;
    case 'v':
        if (ModoDeVisao < 2)
        {
            ModoDeVisao++;
        }
        else
        {
            ModoDeVisao = 0;
        }
        glutPostRedisplay();
        break;
    default:
        cout << key;
        break;
    }
}

void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        jogadorRotacao += 8.0; // Rotaciona 5 graus para a esquerda
        jogadorVetorRotacao -= 8.0;
        if (jogadorRotacao >= 360.0)
            jogadorRotacao -= 360.0;
        break;
    case GLUT_KEY_LEFT:
        jogadorRotacao -= 8.0; // Rotaciona 5 graus para a direita
        jogadorVetorRotacao += 8.0;
        if (jogadorRotacao < 0.0)
            jogadorRotacao += 360.0;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(700, 700);
    glutCreateWindow("Computacao Grafica - Exemplo Basico 3D");

    init();

    LeMapa("mapaOficial.txt");
    InicializaPosicoesCombustiveis(nroCombustiveis);
    InicializaPosicoesInimigos(nroInimigos);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutIdleFunc(animate);

    glutMainLoop();
    return 0;
}
