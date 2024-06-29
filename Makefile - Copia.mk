# Makefile para Windows

PROG = BasicoOpenGL.exe
#FONTES = Linha.cpp Ponto.cpp Temporizador.cpp InterseccaoEntreTodasAsLinhas.cpp 
FONTES = Inimigo.cpp Ponto.cpp Temporizador.cpp ListaDeCoresRGB.cpp 
OBJETOS = $(FONTES:.cpp=.cpp)
CFLAGS = -g -Iinclude\GL # -Wall -g  # Todas as warnings, infos de debug
#LDFLAGS = -Llib\GL -lfreeglut -lopengl32 -lglu32 -lm
LDFLAGS = -Llib -lfreeglut -lopengl32 -lglu32 -lm
CC = g++

$(PROG): $(OBJETOS)
	g++ $(CFLAGS) $(OBJETOS) -o $@ $(LDFLAGS)

clean:
	rm $(OBJETOS) $(PROG)
