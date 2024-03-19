#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

///HERRAMIENTA DEBUG
typedef struct {
    Color veryDarkTeal;
    Color darkTeal;
    Color teal;
    Color lightTeal;
    Color veryLightTeal;
} ColoresDebug;

typedef struct {
    Rectangle ventana;
    int variableActivada[12];
    float valorVariable[10];
    Rectangle botonVariable[12];
    ColoresDebug color; //Teal
    int mostrarHerramienta; //activa o desactiva la herramienta
} Debug;

int i, j, k;

#include  "debug.c"
#endif // DEBUG_H_INCLUDED
