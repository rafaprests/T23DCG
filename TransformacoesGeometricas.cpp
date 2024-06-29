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

std::vector<std::vector<int>> mapa;
std::vector<std::vector<bool>> rotacaoParede;

Ponto jogador;
std::vector<Inimigo> inimigos;
bool andando = false;

double jogadorRotacao = 0.0; // Ângulo de rotação do jogador em graus
double jogadorVelocidade = 0.1; // Velocidade de movimento do jogador

// energia que o jogador inicializa
int energia = 100;
// valor do reabastecimento que o jogador ganha ao passar por combustivel
int valor_de_reabastecimento = 20;


Temporizador T;
double AccumDeltaT = 0;

GLfloat AspectRatio, angulo = 0;

// Controle do modo de projecao
// 0: Projecao Paralela Ortografica; 1: Projecao Perspectiva
// A funcao "PosicUser" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'p'
int ModoDeProjecao = 1;

// Controle do modo de projecao
// 0: Wireframe; 1: Faces preenchidas
// A funcao "Init" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'e'
int ModoDeExibicao = 1;

// 0: Primeira pessoa, 1: Visão de cima
int ModoDeVisao = 0; 

double nFrames = 0;
double TempoTotal = 0;
Ponto OBS;
Ponto ALVO;
Ponto VetorAlvo;

void LeMapa(const std::string &filename);
void DesenhaParede(float x, float y, float z, bool rotate);
void DesenhaLabirinto();
void AtualizaPosicaoJogador();
void DesenhaJogador();
void init(void);
void animate();
void DefineLuz(void);
void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar);
void PosicUser();
void reshape(int w, int h);
void display(void);
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);
void ColidiuComGas();
void ColidiuComInimigo(int inimigoIndex);

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
    while (getline(file, line))
    {
        std::vector<int> row;
        std::vector<bool> rotRow; // Linha para armazenar rotação das paredes

        std::stringstream ss(line);
        int value;
        while (ss >> value)
        {
            row.push_back(value);
            if (value == WALL || value == WINDOW || value == DOOR)
            {
                // Verifica se precisa rotacionar a parede
                bool rotate = false;
                // Verifica paredes adjacentes
                if (row.size() > 1 && row[row.size() - 2] == WALL)
                    rotate = true;
                else if (!rotRow.empty() && rotRow.back())
                    rotate = true;

                rotRow.push_back(rotate);
            }
            else
            {
                rotRow.push_back(false); // Não rotaciona se não for parede
            }

            if (ss.peek() == ',')
                ss.ignore();
        }
        mapa.push_back(row);
        rotacaoParede.push_back(rotRow); // Armazena a linha de rotação
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

    // Ajuste para desenhar a parede inteira para cima
    glTranslatef(0, alturaParede / 2, 0); // 1.35 metros para cima

    // Verifica se deve rotacionar a parede
    if (rotate)
        glRotatef(90, 0, 1, 0); // Rotaciona 90 graus em torno do eixo y

    // Desenha a parede com a altura correta
    glScalef(1, alturaParede, 0.25); // 2.7 metros de altura e 0.25 metros de espessura

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

    glTranslatef(0, altura/2 , 0); 
    glScalef(altura, altura, altura); 

    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1, 1, 1); 
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaJanela(float x, float y, float z)
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
    glScalef(1, alturaInferiorJanela, espessuraParede);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();

    // Desenhar a janela
    glPushMatrix();
    defineCor(LightBlue);
    glTranslatef(x, y + alturaInferiorJanela + alturaJanela / 2, z);
    glScalef(larguraJanela, alturaJanela, 0.1);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();

    // Desenhar a parede superior
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y + alturaInferiorJanela + alturaJanela + (alturaParede - (alturaInferiorJanela + alturaJanela)) / 2, z);
    glScalef(1, alturaParede - (alturaInferiorJanela + alturaJanela), espessuraParede);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();
}

void DesenhaPorta(float x, float y, float z)
{
    float alturaParede = 2.7;
    float alturaPorta = 2.1;
    float espessuraParede = 0.25;
    float larguraPorta = 1;

    // Desenhar a porta (do chão até 2.1 metros)
    glPushMatrix();
    defineCor(Bronze);
    glTranslatef(x, y + alturaPorta / 2, z);
    glScalef(larguraPorta, alturaPorta, 0.1);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();

    // Desenhar a parede superior (de 2.1 metros até 2.7 metros)
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y + alturaPorta + (alturaParede - alturaPorta) / 2, z);
    glScalef(1, alturaParede - alturaPorta, espessuraParede);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1);
    glPopMatrix();
}

// Função para desenhar os inimigos
void DesenhaInimigos() {
    for (const auto& inimigo : inimigos) {
        glPushMatrix();
        glTranslatef(inimigo.Posicao.x, inimigo.Posicao.y, inimigo.Posicao.z); // Translada para a posição do inimigo
        glRotatef(inimigo.Rotacao, 0.0f, 1.0f, 0.0f); // Rotaciona conforme a rotação do inimigo (eixo y)

        // Desenhe o modelo do inimigo usando comandos OpenGL
        float altura = 2.0;
        glPushMatrix();
        glColor3f(inimigo.r, inimigo.g, inimigo.b); // Define a cor do inimigo
        glTranslatef(0, altura/2 , 0); 
        glScalef(0.4, altura, 0.25); 
        glutSolidCube(1);

        // Adiciona borda preta
        glColor3f(0, 0, 0);
        glScalef(1, 1, 1);
        glutWireCube(1);

        glPopMatrix();
        glPopMatrix();
    }
}
// void DesenhaInimigo(float x, float y, float z)
// {
//     float altura = 2.0;
//     glPushMatrix();
//     defineCor(Black);
//     glTranslatef(x, y, z);

//     glTranslatef(0, altura/2 , 0); 
//     glScalef(0.4, altura, 0.25); 

//     glutSolidCube(1);

//     // Adiciona borda preta
//     glColor3f(0, 0, 0);
//     glScalef(1, 1, 1);
//     glutWireCube(1);

//     glPopMatrix();
// }

void DesenhaCadeira(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

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

void DesenhaMesa(float x, float y, float z) {
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

// Função para inicializar as posições dos inimigos
void InicializaPosicoesInimigos(int quantidade)
{
    srand(time(nullptr)); // Inicializa a semente do gerador de números aleatórios

    for (int i = 0; i < quantidade; ++i)
    {
        int x, z;
        do {
            x = rand() % mapa[0].size(); // Posição aleatória na largura do mapa
            z = rand() % mapa.size();   // Posição aleatória na altura do mapa
        } while (mapa[z][x] != CORRIDOR); // Garante que a posição seja um corredor vazio

        inimigos.emplace_back(0.0f, Ponto(x, 0.0f, z));
    }
}

void DesenhaLabirinto()
{
    for (size_t i = 0; i < mapa.size(); ++i)
    {
        for (size_t j = 0; j < mapa[i].size(); ++j)
        {
            float x = j * 1.0; // Tamanho da célula em metros
            float z = i * 1.0;

            switch (mapa[i][j])
            {
            case CORRIDOR:
                DesenhaPiso(x, 0, z);
                break;
            case WALL:
                DesenhaPiso(x, 0, z);
                if (rotacaoParede[i][j])
                    DesenhaParede(x, 0, z, false);
                else
                    DesenhaParede(x, 0, z, true); // Sem rotação
                break;
            case WINDOW:
                DesenhaPiso(x, 0, z);
                DesenhaJanela(x, 0, z);
                break;
            case DOOR:
                DesenhaPiso(x, 0, z);
                DesenhaPorta(x, 0, z);
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
            // case ENEMY:
            //     DesenhaPiso(x, 0, z);
            //     //DesenhaInimigo(x, 0, z);
            //     inimigos.emplace_back(0.0f, Ponto(x, 0.0f, z));
            //     break;
            case CHAIR:
                DesenhaPiso(x, 0, z);
                DesenhaCadeira(x, 0, z);
                break;
            case TABLE:
                DesenhaPiso(x, 0, z);
                DesenhaMesa(x, 0, z);
                break;
            }
        }
    }
}

void ColidiuComGas()
{
    // Encontra uma posição aleatória válida para o combustível
    int novoX, novoZ;
    do {
        novoX = rand() % mapa[0].size(); // Posição aleatória na largura do mapa
        novoZ = rand() % mapa.size();   // Posição aleatória na altura do mapa
    } while (mapa[novoZ][novoX] != CORRIDOR); // Garante que a posição seja um corredor vazio

    // Move o combustível para a nova posição aleatória
    mapa[novoZ][novoX] = GAS;

    // Aumenta a energia do jogador
    energia += valor_de_reabastecimento;
}

void ColidiuComInimigo(int inimigoIndex)
{
    // Encontra uma posição aleatória válida para o inimigo
    int novoX, novoZ;
    do {
        novoX = rand() % mapa[0].size(); // Posição aleatória na largura do mapa
        novoZ = rand() % mapa.size();   // Posição aleatória na altura do mapa
    } while (mapa[novoZ][novoX] != CORRIDOR); // Garante que a posição seja um corredor vazio

    // Atualiza a posição do inimigo no vetor
    inimigos[inimigoIndex].Posicao.x = novoX;
    inimigos[inimigoIndex].Posicao.z = novoZ;

    // Reduz a energia do jogador (exemplo)
    energia -= 10; // Ajuste conforme necessário
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

        if (mapa[mapaZ][mapaX] == CORRIDOR || mapa[mapaZ][mapaX] == GAS || mapa[mapaZ][mapaX] == ENEMY) 
        {
            if (mapa[mapaZ][mapaX] == GAS){
                ColidiuComGas();
            }

            // Remove o item colidido do mapa, se for GAS
            if (mapa[mapaZ][mapaX] == GAS) {
                mapa[mapaZ][mapaX] = CORRIDOR;
            }
            jogador.x = novoX;
            jogador.z = novoZ;
        }
    }
    // Atualizar o vetor alvo de acordo com a nova direção do jogador
    VetorAlvo.x = cos(rad);
    VetorAlvo.z = sin(rad);
    VetorAlvo.y = 0; // Manter o alvo no plano 
}

void MoveInimigos() {
    float velocidade = 20.0; // Velocidade dos inimigos em metros por segundo
    float dt = T.getDeltaT(); // Obter o tempo delta para um movimento suave

    for (size_t i = 0; i < inimigos.size(); ++i) {
        auto& inimigo = inimigos[i];

        // Calcular vetor direção do inimigo em direção ao jogador
        float deltaX = jogador.x - inimigo.Posicao.x;
        float deltaZ = jogador.z - inimigo.Posicao.z;
        float distancia = sqrt(deltaX * deltaX + deltaZ * deltaZ);

        // Normalizar o vetor direção
        float dirX = deltaX / distancia;
        float dirZ = deltaZ / distancia;

        // Calcular a nova posição do inimigo com base na velocidade constante
        float novoX = inimigo.Posicao.x + dirX * velocidade * dt;
        float novoZ = inimigo.Posicao.z + dirZ * velocidade * dt;

        // Verificar colisões com o jogador
        int mapaX = static_cast<int>(novoX + 0.5);
        int mapaZ = static_cast<int>(novoZ + 0.5);

        if (static_cast<int>(jogador.x + 0.5) == mapaX && static_cast<int>(jogador.z + 0.5) == mapaZ) {
            // Se colidiu com o jogador, chama a função de colisão
            ColidiuComInimigo(i);
            break;
        } else if (mapa[mapaZ][mapaX] == CORRIDOR) {
            // Atualizar a posição do inimigo se não houver colisão com obstáculos
            inimigo.Posicao.x = novoX;
            inimigo.Posicao.z = novoZ;
        }
    }
}

void DesenhaJogador()
{
    glPushMatrix();
    glTranslatef(jogador.x, jogador.y, jogador.z);
    glRotatef(jogadorRotacao, 0.0, 1.0, 0.0); // Aplica a rotação do jogador
    glColor3f(1, 0, 0);           // Cor vermelha para o jogador
    glutSolidSphere(0.5, 20, 20); // Desenha o jogador como uma esfera
    glPopMatrix();
}

void init(void)
{
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // Fundo de tela preto

    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade
    glClearDepth(1.0);        // Limpa o valor do Z-buffer para o valor máximo
    glDepthFunc(GL_LESS);     // Função de teste de profundidade
    glEnable(GL_CULL_FACE);   // Habilita o culling de faces
    glEnable(GL_NORMALIZE);   // Normaliza normais para cálculos corretos de iluminação
    glShadeModel(GL_FLAT);    // Modelo de sombreamento flat para um efeito mais simples

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    if (ModoDeExibicao) // Faces Preenchidas??
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
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
        if (inimigos.size() != 0){
            MoveInimigos();
        }
        glutPostRedisplay();
    }
}

void DefineLuz(void)
{
    // Define cores para um objeto dourado
    // GLfloat LuzAmbiente[]   = {0.0, 0.0, 0.0 } ;
    GLfloat LuzAmbiente[] = {0.4, 0.4, 0.4};
    GLfloat LuzDifusa[] = {0.7, 0.7, 0.7};
    // GLfloat LuzDifusa[]   = {0, 0, 0};
    GLfloat PosicaoLuz0[] = {0.0f, 3.0f, 5.0f}; // Posi��o da Luz
    GLfloat LuzEspecular[] = {0.9f, 0.9f, 0.9};
    // GLfloat LuzEspecular[] = {0.0f, 0.0f, 0.0 };

    GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};

    // ****************  Fonte de Luz 0

    glEnable(GL_COLOR_MATERIAL);

    // Habilita o uso de ilumina��o
    glEnable(GL_LIGHTING);

    // Ativa o uso da luz ambiente
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
    // Define os parametros da luz n�mero Zero
    glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0);
    glEnable(GL_LIGHT0);

    // Ativa o "Color Tracking"
    glEnable(GL_COLOR_MATERIAL);

    // Define a reflectancia do material
    glMaterialfv(GL_FRONT, GL_SPECULAR, Especularidade);

    // Define a concentra��oo do brilho.
    // Quanto maior o valor do Segundo parametro, mais
    // concentrado ser� o brilho. (Valores v�lidos: de 0 a 128)
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
     // Define os par�metros da proje��o Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define o volume de visualiza��o sempre a partir da posicao do
    // observador
    if (ModoDeProjecao == 0)
        glOrtho(-10, 10, -10, 10, 0, 20); // Projecao paralela Orthografica
    else
        MygluPerspective(60, AspectRatio, 0.1, 50); // Projecao perspectiva

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

     if (ModoDeVisao == 0) // Primeira pessoa
    {
        gluLookAt(jogador.x, jogador.y + 1.5, jogador.z, // Posiçao do observador
                  jogador.x + VetorAlvo.x, jogador.y + 1.5 + VetorAlvo.y, jogador.z + VetorAlvo.z, // Posiçao do alvo
                  0.0, 1.0, 0.0);
    }
    else if (ModoDeVisao == 1) // Visão de cima
    {
        gluLookAt(jogador.x, 20, jogador.z, // Posiçao do observador
                  jogador.x, 0, jogador.z,  // Posiçao do alvo
                  0.0, 0.0, -1.0);          // Up vector (direção para baixo)
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

    DefineLuz();
    PosicUser();

    glMatrixMode(GL_MODELVIEW);
    DesenhaLabirinto();
    DesenhaJogador();
    DesenhaInimigos();

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
        andando = !andando;
        break;
    case 'p':
        ModoDeProjecao = !ModoDeProjecao;
        glutPostRedisplay();
        break;
    case 'e':
        ModoDeExibicao = !ModoDeExibicao;
        init();
        glutPostRedisplay();
        break;
    case 'v':
        ModoDeVisao = !ModoDeVisao;
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
        jogadorRotacao += 5.0; // Rotaciona 5 graus para a esquerda
        if (jogadorRotacao >= 360.0)
            jogadorRotacao -= 360.0;
        break;
    case GLUT_KEY_LEFT:
        jogadorRotacao -= 5.0; // Rotaciona 5 graus para a direita
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

    LeMapa("mapa.txt");
    InicializaPosicoesInimigos(10);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutIdleFunc(animate);

    glutMainLoop();
    return 0;
}
