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

// Define constants for map elements
#define CORRIDOR 0
#define WALL 1
#define WINDOW 2
#define DOOR 3
#define PLAYER_START 4
#define GAS 5
#define ENEMY 6

std::vector<std::vector<int>> mapa;

Ponto jogador;
bool andando = false;

double jogadorRotacao = 0.0; // Ângulo de rotação do jogador em graus
double jogadorVelocidade = 0.1; // Velocidade de movimento do jogador


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
void DesenhaParede(float x, float y, float z);
void DesenhaLabirinto();
void AtualizaPosicaoJogador();
void DesenhaJogador();
void init(void);
void animate();
void DesenhaCubo(float tamAresta);
void DesenhaParalelepipedo();
void DesenhaLadrilho(int corBorda, int corDentro);
void DefineLuz(void);
void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar);
void PosicUser();
void reshape(int w, int h);
void display(void);
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);

// Função para ler o mapa do arquivo
void LeMapa(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo do mapa!" << endl;
        exit(1);
    }
    else
    {
        cout << "arquivo do mapa aberto" << endl;
    }

    std::string line;
    while (getline(file, line))
    {
        cout << "Linha lida: " << line << endl;
        std::vector<int> row;
        std::stringstream ss(line);
        int value;
        while (ss >> value)
        {
            row.push_back(value);
            if (ss.peek() == ',')
                ss.ignore();
            cout << "Valor lido: " << value << endl;
        }
        mapa.push_back(row);
    }

    file.close();

    for (const auto &row : mapa)
    {
        for (int cell : row)
        {
            cout << cell << " ";
        }
        cout << endl;
    }
}

void DesenhaParede(float x, float y, float z)
{
    float alturaParede = 2.7;
    glPushMatrix();
    defineCor(Copper);
    glTranslatef(x, y, z);

    // Ajuste para desenhar a parede inteira para cima
    glTranslatef(0, alturaParede/2 , 0); // 1.35 metros para cima

    // Desenha a parede com a altura correta
    glScalef(1, alturaParede, 0.25); // 2.7 metros de altura e 0.25 metros de espessura

    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1.01, 1.01, 1.01); // Ligeiramente maior para desenhar a borda
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
    glScalef(1.01, 1.01, 1.01); // Ligeiramente maior para desenhar a borda
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
    glScalef(1.01, 1.01, 1.01); // Ligeiramente maior para desenhar a borda
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaJanela(float x, float y, float z)
{
    float altura = 1.8;
    glPushMatrix();
    defineCor(LightBlue);
    glTranslatef(x, y, z);

    glTranslatef(0, altura/2 + 0.9 , 0); 
    glScalef(1, altura, 0.25); 

    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1.01, 1.01, 1.01); // Ligeiramente maior para desenhar a borda
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaPorta(float x, float y, float z)
{
    float altura = 2.1;
    glPushMatrix();
    defineCor(Bronze);
    glTranslatef(x, y, z);

    glTranslatef(0, altura/2 , 0); 
    glScalef(1, altura, 0.25); 

    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1.01, 1.01, 1.01); // Ligeiramente maior para desenhar a borda
    glutWireCube(1);

    glPopMatrix();
}

void DesenhaInimigo(float x, float y, float z)
{
    float altura = 2.0;
    glPushMatrix();
    defineCor(Black);
    glTranslatef(x, y, z);

    glTranslatef(0, altura/2 , 0); 
    glScalef(0.4, altura, 0.25); 

    glutSolidCube(1);

    // Adiciona borda preta
    glColor3f(0, 0, 0);
    glScalef(1.01, 1.01, 1.01); // Ligeiramente maior para desenhar a borda
    glutWireCube(1);

    glPopMatrix();
}



void DesenhaLabirinto()
{
    for (size_t i = 0; i < mapa.size(); ++i)
    {
        for (size_t j = 0; j < mapa[i].size(); ++j)
        {
            float x = j * 1.0; // tamanho da célula em metros
            float z = i * 1.0;

            switch (mapa[i][j])
            {
            case CORRIDOR:
                DesenhaPiso(x,0,z);
                break;
            case WALL:
                DesenhaPiso(x,0,z);
                DesenhaParede(x, 0, z); 
                break;
            case WINDOW:
                DesenhaPiso(x,0,z);
                DesenhaJanela(x, 0, z);
                break;
            case DOOR:
                DesenhaPiso(x,0,z);
                DesenhaPorta(x, 0, z);
                break;
            case PLAYER_START:
                mapa[i][j] = CORRIDOR;
                DesenhaPiso(x,0,z);
                jogador.x = x;
                jogador.y = 0;
                jogador.z = z;
                break;
            case GAS:
                DesenhaPiso(x,0,z);
                DesenhaCombustivel(x,0,z);
                break;
            case ENEMY:
                DesenhaPiso(x,0,z);
                DesenhaInimigo(x,0,z);
                break;
            }
        }
    }
}

void AtualizaPosicaoJogador()
{
    // Converter o ângulo de rotação de graus para radianos
    double rad = jogadorRotacao * M_PI / 180.0;
    if (andando)
    {
         // Calcula a próxima posição do jogador
        double nextX = jogador.x + jogadorVelocidade * cos(rad);
        double nextZ = jogador.z + jogadorVelocidade * sin(rad);

        // Verifica se a próxima posição não é uma parede (WALL)
        int cellX = static_cast<int>(nextX);
        int cellZ = static_cast<int>(nextZ);
        
        if (mapa[cellZ][cellX] != WALL)
        {
            // Se não for parede, atualiza a posição do jogador
            jogador.x = nextX;
            jogador.z = nextZ;
        }
    }
    // Atualizar o vetor alvo de acordo com a nova direção do jogador
    VetorAlvo.x = cos(rad);
    VetorAlvo.z = sin(rad);
    VetorAlvo.y = 0; // Manter o alvo no plano 

    // Implementar colisões e outras interações
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
        glutPostRedisplay();
    }
}

void DesenhaCubo(float tamAresta)
{
    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0, 0, 1);
    glVertex3f(-tamAresta / 2, -tamAresta / 2, tamAresta / 2);
    glVertex3f(tamAresta / 2, -tamAresta / 2, tamAresta / 2);
    glVertex3f(tamAresta / 2, tamAresta / 2, tamAresta / 2);
    glVertex3f(-tamAresta / 2, tamAresta / 2, tamAresta / 2);
    // Back Face
    glNormal3f(0, 0, -1);
    glVertex3f(-tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
    glVertex3f(-tamAresta / 2, tamAresta / 2, -tamAresta / 2);
    glVertex3f(tamAresta / 2, tamAresta / 2, -tamAresta / 2);
    glVertex3f(tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
    // Top Face
    glNormal3f(0, 1, 0);
    glVertex3f(-tamAresta / 2, tamAresta / 2, -tamAresta / 2);
    glVertex3f(-tamAresta / 2, tamAresta / 2, tamAresta / 2);
    glVertex3f(tamAresta / 2, tamAresta / 2, tamAresta / 2);
    glVertex3f(tamAresta / 2, tamAresta / 2, -tamAresta / 2);
    // Bottom Face
    glNormal3f(0, -1, 0);
    glVertex3f(-tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
    glVertex3f(tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
    glVertex3f(tamAresta / 2, -tamAresta / 2, tamAresta / 2);
    glVertex3f(-tamAresta / 2, -tamAresta / 2, tamAresta / 2);
    // Right face
    glNormal3f(1, 0, 0);
    glVertex3f(tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
    glVertex3f(tamAresta / 2, tamAresta / 2, -tamAresta / 2);
    glVertex3f(tamAresta / 2, tamAresta / 2, tamAresta / 2);
    glVertex3f(tamAresta / 2, -tamAresta / 2, tamAresta / 2);
    // Left Face
    glNormal3f(-1, 0, 0);
    glVertex3f(-tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
    glVertex3f(-tamAresta / 2, -tamAresta / 2, tamAresta / 2);
    glVertex3f(-tamAresta / 2, tamAresta / 2, tamAresta / 2);
    glVertex3f(-tamAresta / 2, tamAresta / 2, -tamAresta / 2);
    glEnd();
}
void DesenhaParalelepipedo()
{
    glPushMatrix();
    glTranslatef(0, 0, -1);
    glScalef(1, 1, 2);
    glutSolidCube(2);
    // DesenhaCubo(1);
    glPopMatrix();
}

// **********************************************************************
// void DesenhaLadrilho(int corBorda, int corDentro)
// Desenha uma c�lula do piso.
// Eh possivel definir a cor da borda e do interior do piso
// O ladrilho tem largula 1, centro no (0,0,0) e est� sobre o plano XZ
// **********************************************************************
void DesenhaLadrilho(int corBorda, int corDentro)
{
    //defineCor(Pink); // desenha QUAD preenchido
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0.0f, -0.5f);
    glVertex3f(-0.5f, 0.0f, 0.5f);
    glVertex3f(0.5f, 0.0f, 0.5f);
    glVertex3f(0.5f, 0.0f, -0.5f);
    glEnd();

    //defineCor(GreenYellow);
    glColor3f(0,1,0);

    glBegin(GL_LINE_STRIP);
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0.0f, -0.5f);
    glVertex3f(-0.5f, 0.0f, 0.5f);
    glVertex3f(0.5f, 0.0f, 0.5f);
    glVertex3f(0.5f, 0.0f, -0.5f);
    glEnd();
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

    LeMapa("mapa.txt");
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutIdleFunc(animate);

    glutMainLoop();
    return 0;
}
