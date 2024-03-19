///LIBRERÍAS
#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include "raylib.h"
#include <math.h>
#include <string.h>
#include "debug.h"
///PANTALLA
#define SCREENWIDTH 1920
#define SCREENHEIGHT 1080
///INDEX STATE MACHINE Y ESCENARIO
#define DIM 30
#define HABITACION 0
#define COCINA 1
#define JARDIN 2
#define MENU 3
#define PAUSA 4
#define SAVE 5
#define TEXTO 5
#define SCORE 6
#define CREDITOS 7
#define GANASTE 8
#define GAMEOVER 10
#define DENTROTEXTO 29
#define FUERATEXTO 30
///INDEX OBJETOS HABITACIÓN 1
#define MUNIECA 0
#define ALFOMBRA 1
#define ESPEJO 2
#define ZAPATOS 3
#define GUITARRA 4
#define CAMA 5
#define VENTANA 6
#define CAJON 7
#define ALMOHADA 8
#define ARMARIO 9
#define CANDADOPUERTA 10
#define FONDOVENTANA1 11
#define FONDOVENTANA2 12
#define FONDOVENTANA3 13
#define FONDOVENTANA4 14
#define OREJA 17
#define FONDOESPEJO1 15
#define FONDOESPEJO2 16
#define PUERTA 20
///INDEX OBJETOS COCINA
#define CUADRO 0
#define TACHO 1
#define HELADERA 2
#define CUCHILLO 3
#define PARED 4
#define MESADA 5
//#define PUERTA 6
///INDEX OBJETOS JARDÍN
///BOTONES DEL MENÚ
#define PLAY 0
#define BOTONLOAD 1
#define BOTONSCORE 2
#define BOTONSAVE 3
#define BOTONCREDITS 4
#define BOTONGANASTE 5
#define BOTONSCOREBACK 6
///BOTONES DE CREDITOS
#define BACK 0
///CONSTANTES
const char nombreArchivoSave[DIM] = "EscapeAliveScores.bin";

///ESTRUCTURAS
typedef struct stSonidos {
    Sound musica;
    Sound efectos[DIM];
} stSonidos;

typedef struct stObjeto {
    Texture2D sprite[DIM];
    Rectangle hitbox;           ///El hitbox contiene la posición
    int existe;
} stObjeto;

typedef struct stEvento {
    int idObjeto;
    int idProgreso;
    int idEvento;             ///Indice de nodo de los objetos fijo para buscar
    int activo;                 ///Existe 0 o 1
    char texto[DIM*10];
    Sound sonido;
} stEvento;                     ///camino dentro del arbol

typedef struct stNodo {
    stEvento evento;
    struct stNodo *anterior;
    struct stNodo *siguiente;
} stNodo;

typedef struct stFila {
    stNodo *primero;
    stNodo *ultimo;
} stFila;

typedef struct stArbol {
    stFila *fila;
    struct stArbol *izquierda;
    struct stArbol *derecha;
} stArbol;

typedef struct stEscenario {
    Texture2D fondoEstatico;
    Texture2D subEscenarios[DIM];
    Texture2D botones[DIM];
    stArbol arbolDelEscenario;
    stObjeto objetos[DIM];
} stEscenario;

typedef struct stFlag {
    int idProgreso;
    int idEvento;
    int itemObtenido;
} stFlag;

typedef struct stJugador {
    stFlag flags[DIM]; //Booleanos relacionados a eventos
    int idObjeto;
    int ultimoObjetoClickeado;
    stArbol *posicionSubArbol; //posicion actual en el arbol
    int escenario; //Número de escenario o subescenario (state machine)
    int textoActivo;
    int subEscenarioActivo;
    char nombre[DIM];
    int score;
    int scrollText; ///inicializar
    Sound gameOverSound;
    int switchTimer;
    int switchState;
    int saveGuardado;
    int posicionLetra;
} stJugador;

///VARIABLES GLOBALES CONTADORAS
int i, j, k;

///MAIN
int main() {

    srand(time(NULL));
    stSonidos sonidos;
    stJugador *player = (stJugador *)malloc(sizeof(stJugador));
    stEscenario escenario[DIM]; ///TOQUE

    Debug herramientaDebug;
    inicializaciones(&sonidos, player, escenario, &herramientaDebug);

    ///-------------------------------------------------------------------------------------
    ///BUCLE PRINCIPAL
    ///-------------------------------------------------------------------------------------
    while( !WindowShouldClose() ) {
        actualizar(escenario, player, &herramientaDebug);
        dibujar(escenario, player, &herramientaDebug);
    }
    ///-------------------------------------------------------------------------------------
    ///DE-INICIALIZACION
    ///-------------------------------------------------------------------------------------
    deinicializaciones();
    return 0;
}
///FUNCIONES
///INICIALIZACIONES
void inicializaciones(stSonidos *sonidos, stJugador *player, stEscenario *escenario, Debug *herramientaDebug) {
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Escape Alive");
    SetTargetFPS(60);
    InitAudioDevice();
    inicializarSonidos(sonidos);
    inicializarJugador(player);
    inicializarEscenarios(escenario);
    if( elArchivoExiste() == 0 ) inicializarSavesDefault();
    inicializarDebug(herramientaDebug);
    mostrarArbol(escenario->arbolDelEscenario); ///debug
}
int elArchivoExiste() {
    int existe = 0;
    FILE *bufferArchivo = fopen(nombreArchivoSave,"rb");
    if(bufferArchivo != NULL) existe = 1;
    return existe;
}
void inicializarSonidos(stSonidos *sonidos) {
    //sonidos->musica = loadSoundRaylib();
    //sonidos->efectos[DIM] = loadEffectRaylib();
}
void inicializarJugador(stJugador *player) {
    for( i = 0 ; i < DIM ; i++ ){ ///Inicializamos todos los flags en 0
        (*player).flags[i].idProgreso = 0;
        (*player).flags[i].idEvento = 0; ///PROBABLEMENTE NO LO USEMOS CUANDO HAGAMOS LAS FUNCIONES DE FILAS
    }
    player->idObjeto = -1;
    player->ultimoObjetoClickeado = -1;
    player->posicionSubArbol = NULL;
    player->nombre[0] = '0/';
    player->score = 54000; ///10 min a 60 frames = 54000
    player->escenario = MENU;
    player->textoActivo = 0;
    player->subEscenarioActivo = 0;
    player->scrollText = 0;
    player->gameOverSound = LoadSound("gameOver.mp3");
}
void inicializarEscenarios(stEscenario *escenario) {
    inicializarMenu(escenario);
    inicializarTextos(escenario);
    inicializarHabitacion(escenario);
    //inicializarCocina(escenario);
    //inicializarJardin(escenario);
    inicializarCreditos(escenario);
    inicializarGanaste(escenario); ///TQOUE ACA
    inicializarSave(escenario); ///TOQUE ACA
    inicializarScore(escenario); ///TOQUE ACA
    inicializarGameOver(escenario); ///TOQUE ACA
}
void inicializarSavesDefault() {
    stJugador players[5];
    inicializarNombresSavesDefault(players);
    inicializarValoresSavesDefault(players);
    sobreescribirArchivoSavesDefault(players);
}
void inicializarNombresSavesDefault(stJugador *player) {
    char nombreScore1[DIM] = "ElMuerto"; ///INVENTAR NOMBRES NUEVOS
    strcpy(player[0].nombre, nombreScore1);
    char nombreScore2[DIM] = "Fantasma";
    strcpy(player[1].nombre, nombreScore2);
    char nombreScore3[DIM] = "No Soy La Munieca";
    strcpy(player[2].nombre, nombreScore3);
    char nombreScore4[DIM] = "Demonio666";
    strcpy(player[3].nombre, nombreScore4);
    char nombreScore5[DIM] = "Los duendes";
    strcpy(player[4].nombre, nombreScore5);
}
void inicializarValoresSavesDefault(stJugador player[]) {
    player[0].score = 46000;
    player[1].score = 41500;
    player[2].score = 39700;
    player[3].score = 24800;
    player[4].score = 16400;
    for( i = 0 ; i < 5 ; i++ ) {
        for( j = 0 ; j < DIM ; j++ ){ ///Inicializamos todos los flags en 0
            player[i].flags[j].idProgreso = 0;
            player[i].flags[j].idEvento = 0; ///PROBABLEMENTE NO LO USEMOS CUANDO HAGAMOS LAS FUNCIONES DE FILAS
        }
        player[i].idObjeto = -1;
        player[i].ultimoObjetoClickeado = -1;
        player[i].posicionSubArbol = NULL;
        player[i].nombre[0] = '0/';
        player[i].score = 54000; ///15 min a 60 frames = 54000
        player[i].escenario = MENU;
        player[i].textoActivo = 0;
        player[i].subEscenarioActivo = 0;
        player[i].scrollText = 0;
        player[i].gameOverSound = LoadSound("gameOver.mp3");
    }
}
void sobreescribirArchivoSavesDefault(stJugador *players) {
    int validosSave = 5;
    FILE *archivoSave = fopen(nombreArchivoSave, "wb");
    if( archivoSave == NULL ) printf("\nError. El archivo no existe.");
    if( archivoSave != NULL ) {
        for( i = 0 ; i < validosSave ; i++ )
            fwrite(&players[i], sizeof(stJugador), 1, archivoSave);
        fclose(archivoSave);
    }
}
///Habitación
void inicializarTextos(stEscenario *escenario) {
    for( i = 0 ; i < 3 ; i++) {
        escenario[i].objetos[DENTROTEXTO].existe = 0;
        escenario[i].objetos[DENTROTEXTO].hitbox.x = 500;
        escenario[i].objetos[DENTROTEXTO].hitbox.y = 750;
        escenario[i].objetos[DENTROTEXTO].hitbox.width = 1030;
        escenario[i].objetos[DENTROTEXTO].hitbox.height = 279;
        escenario[i].objetos[FUERATEXTO].existe = 0;
        escenario[i].objetos[FUERATEXTO].hitbox.x = 500;
        escenario[i].objetos[FUERATEXTO].hitbox.y = 0;
        escenario[i].objetos[FUERATEXTO].hitbox.width = 1030;
        escenario[i].objetos[FUERATEXTO].hitbox.height = 750;
        escenario[i].subEscenarios[5] = LoadTexture("marcoTextos.png");
    }
}
void inicializarFila(stFila **fila) {
    *fila = (stFila *)malloc(sizeof(stFila));
    (*fila)->primero = NULL;
    (*fila)->ultimo = NULL;
}
void inicializarArbol(stArbol *arbol) {
    arbol->derecha = NULL;
    arbol->izquierda = NULL;
}
void inicializarHabitacion(stEscenario *escenario) {
    escenario[HABITACION].fondoEstatico =    LoadTexture("habitacion.png");
    escenario[HABITACION].subEscenarios[0] = LoadTexture("espejosubescenario.png");
    escenario[HABITACION].subEscenarios[1] = LoadTexture("ventanasubescenario.png");
    inicializarEventosHabitacion(escenario);
    inicializarObjetosHabitacion(escenario);
}
void inicializarEventosHabitacion(stEscenario *escenario) {
    inicializarEventosHabitacionMunieca(escenario);
    inicializarEventosHabitacionAlfombra(escenario);
    inicializarEventosHabitacionCajon(escenario);
    inicializarEventosHabitacionZapatos(escenario);
    inicializarEventosHabitacionGuitarra(escenario);
    inicializarEventosHabitacionArmario(escenario);
    inicializarEventosHabitacionEspejo(escenario);
    inicializarEventosHabitacionVentana(escenario);
    inicializarEventosHabitacionCama(escenario);
    inicializarEventosHabitacionAlmohada(escenario);
    inicializarEventosHabitacionPuerta(escenario);
}
///INICIO ÁRBOL MUNIECA
void inicializarEventosHabitacionMunieca(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///LA REFERENCIA APUNTA A LA RAIZ

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol); ///BORRAR

    ///RAIZ PRINCIPAL
    inicializarArbol(referenciaSubArbol);

    inicializarEventosHabitacionMunieca0(referenciaSubArbol); ///Inerte

    printf("\n RAIZ \n"); ///debug
    mostrarFila((*referenciaSubArbol).fila); ///debug

    ///PROGRESO 0 Inerte
    stArbol inerteMunieca0;
    nuevoSubArbol = &inerteMunieca0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca1(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ DEL ARBOL AL SUBARBOL NIVEL 1 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 1 MUNIECA

    printf("\nPRINT REFERENCIA SUB ARBOL EN DERECHA %p\n", referenciaSubArbol); ///BORRAR

    ///PROGRESO 0 Activo (Desencadena eventos)
    stArbol activoMunieca0;
    nuevoSubArbol = &activoMunieca0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMuniecaProgreso0(nuevoSubArbol);

    printf("\n PROGRESO 0 ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 IZQUIERDA MUNIECA

    ///PROGRESO 1 Inerte
    stArbol inerteMunieca1;
    nuevoSubArbol = &inerteMunieca1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca5(nuevoSubArbol); ///inerte    progreso 1

    printf("\n PROGRESO 1 INERTE \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 MUNIECA

    ///PROGRESO 1 Activo (Desencadena eventos)
    stArbol activoMunieca1;
    nuevoSubArbol = &activoMunieca1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca6(nuevoSubArbol);

    printf("\n PROGRESO 1 activo \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 2 AL SUBARBOL NIVEL 3 IZQUIERDA MUNIECA

    ///PROGRESO 2 Inerte
    stArbol inerteMunieca2;
    nuevoSubArbol = &inerteMunieca2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca7(nuevoSubArbol); ///inerte    progreso 1

    printf("\n PROGRESO 2 INERTE \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 MUNIECA

    ///PROGRESO 2 Activo (Desencadena eventos)
    stArbol activoMunieca2;
    nuevoSubArbol = &activoMunieca2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMuniecaProgreso2(nuevoSubArbol);

    printf("\n PROGRESO 2 activo \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 2 AL SUBARBOL NIVEL 3 IZQUIERDA MUNIECA

    ///PROGRESO 3 Inerte
    stArbol inerteMunieca3;
    nuevoSubArbol = &inerteMunieca3;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca12(nuevoSubArbol); ///inerte    progreso 1

    printf("\n PROGRESO 3 INERTE \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 MUNIECA

    ///PROGRESO 3 Activo (Desencadena eventos)
    stArbol activoMunieca3;
    nuevoSubArbol = &activoMunieca3;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMuniecaProgreso3(nuevoSubArbol);

    printf("\n PROGRESO 3 activo \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 2 AL SUBARBOL NIVEL 3 IZQUIERDA MUNIECA
}
void inicializarEventosHabitacionMuniecaProgreso0(stArbol *subArbol) {
    inicializarEventosHabitacionMunieca2(subArbol);
    inicializarEventosHabitacionMunieca3(subArbol);
    inicializarEventosHabitacionMunieca4(subArbol);
}
void inicializarEventosHabitacionMuniecaProgreso2(stArbol *subArbol) {
    inicializarEventosHabitacionMunieca8(subArbol);
    inicializarEventosHabitacionMunieca9(subArbol);
    inicializarEventosHabitacionMunieca10(subArbol);
    inicializarEventosHabitacionMunieca11(subArbol);
}
void inicializarEventosHabitacionMuniecaProgreso3(stArbol *subArbol) {
    inicializarEventosHabitacionMunieca13(subArbol);
    inicializarEventosHabitacionMunieca14(subArbol);
    inicializarEventosHabitacionMunieca15(subArbol);
}
void inicializarEventosHabitacionMunieca0(stArbol *subArbol) { ///INACTIVO
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 0;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca1(stArbol *subArbol) { ///INACTIVO
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 1;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca2(stArbol *subArbol) { ///PRIMERA SECUENCIA DE EVENTOS
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 2;
    eventoMunieca.activo = 1;
    char texto[] = { "La munieca yace inerte, su rostro de porcelana estatico, los ojos apagados. Algo en su interior parece haber perdido su brillo. Hay algo que puedas hacer para despertarla?" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca3(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 3;
    eventoMunieca.activo = 1;
    char texto[] = { "La munieca revela un hueco en la espalda, como si esperara pilas para cobrar vida." };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca4(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 4;
    eventoMunieca.activo = 1;
    char texto[] = { "SONIDO Revivela" };
    strcpy(eventoMunieca.texto, texto);

    eventoMunieca.sonido = LoadSound("Revivela.mp3");

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca5(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 1;
    eventoMunieca.idEvento = 5;
    eventoMunieca.activo = 1;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca6(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 1;
    eventoMunieca.idEvento = 6;
    eventoMunieca.activo = 1;
    char texto[] = { "El toque despierta su misterio, pero sin pilas, su encanto queda en la sombra. Aguarda, lista para cobrar vida con la energIa adecuada." };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca7(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 2;
    eventoMunieca.idEvento = 7;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca8(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 2;
    eventoMunieca.idEvento = 8;
    eventoMunieca.activo = 0;
    char texto[] = { "Las pilas encajan perfectamente en la munieca, pero la quietud persiste." };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca9(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 2;
    eventoMunieca.idEvento = 9;
    eventoMunieca.activo = 0;
    char texto[] = { "Al observar detenidamente, descubres que la munieca esconde un boton oculto. Prueba encenderla y revelar los secretos." };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca10(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 2;
    eventoMunieca.idEvento = 10;
    eventoMunieca.activo = 0;
    char texto[] = { "La munieca comienza a emitir sonidos desde su boca, suenan tenue. Por que no te acercas?" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca11(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 2;
    eventoMunieca.idEvento = 11;
    eventoMunieca.activo = 0;
    char texto[] = { "SONIDO  Risa macabra" };
    strcpy(eventoMunieca.texto, texto);

    eventoMunieca.sonido = LoadSound("risaMunieca.mp3");

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca12(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 3;
    eventoMunieca.idEvento = 12;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca13(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 3;
    eventoMunieca.idEvento = 13;
    eventoMunieca.activo = 0;
    char texto[] = { "En el armario hay una llave para tu liberacion" };
    strcpy(eventoMunieca.texto, texto);

    eventoMunieca.sonido = LoadSound("armarioLlave.mp3");

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca14(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 3;
    eventoMunieca.idEvento = 14;
    eventoMunieca.activo = 0;
    char texto[] = { "Mientras más te demores en salir, más criaturas iran despertando" };
    strcpy(eventoMunieca.texto, texto);

    eventoMunieca.sonido = LoadSound("MientrasTeDemores .mp3");

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
void inicializarEventosHabitacionMunieca15(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 3;
    eventoMunieca.idEvento = 15;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    eventoMunieca.sonido = LoadSound("grito.mp3");

    agregarNodoEnFila(subArbol->fila, eventoMunieca);
}
///FIN ÁRBOL MUNIECA
///INICIO ÁRBOL ALFOMBRA
void inicializarEventosHabitacionAlfombra(stEscenario *escenario) {
    printf("\ ARBOL ALFOMBRA \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///LA REFERENCIA APUNTA A LA RAIZ

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol); ///BORRAR

    ///RAIZ ALFOMBRA
    stArbol inerteAlfombra0;
    nuevoSubArbol = &inerteAlfombra0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlfombra20(nuevoSubArbol); ///Inerte

    printf("\n RAIZ ALFOMBRA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///ALFOMBRA
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 Inerte
    stArbol inerteAlfombra1;
    nuevoSubArbol = &inerteAlfombra1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlfombra21(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ DEL ARBOL AL SUBARBOL NIVEL 1 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 1 MUNIECA

    ///PROGRESO 0 Activo (Desencadena eventos)
    stArbol activoAlfombra0;
    nuevoSubArbol = &activoAlfombra0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlfombraProgreso0(nuevoSubArbol);

    printf("\n PROGRESO 0 ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 IZQUIERDA MUNIECA

    ///PROGRESO 1 Inerte
    stArbol inerteAlfombra2;
    nuevoSubArbol = &inerteAlfombra2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlfombra25(nuevoSubArbol); ///inerte    progreso 1

    printf("\n PROGRESO 1 INERTE \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 MUNIECA

    ///PROGRESO 1 Activo (Desencadena eventos)
    stArbol activoAlfombra1;
    nuevoSubArbol = &activoAlfombra1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlfombra26(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    printf("\n\MOSTRANDO FIN DEBUG ALFOMBRA \n");
}
void inicializarEventosHabitacionAlfombraProgreso0(stArbol *subArbol) {
    inicializarEventosHabitacionAlfombra22(subArbol);
    inicializarEventosHabitacionAlfombra23(subArbol);
    inicializarEventosHabitacionAlfombra24(subArbol);
}
void inicializarEventosHabitacionAlfombra20(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 20;
    eventoAlfombra.activo = 0;
    char texto[] = { "INERTE 0" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionAlfombra21(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 21;
    eventoAlfombra.activo = 0;
    char texto[] = { "INERTE 1" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionAlfombra22(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 22;
    eventoAlfombra.activo = 1;
    char texto[] = { "La alfombra revela un misterio bajo su suavidad." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionAlfombra23(stArbol *subArbol) {
    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 23;
    eventoAlfombra.activo = 1;
    char texto[] = { "Despliegas la alfombra y descubres un secreto oculto." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionAlfombra24(stArbol *subArbol) {
    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 24;
    eventoAlfombra.activo = 1;
    char texto[] = { "Unas pilas asoman tImidamente, como si esperaran ser encontradas." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionAlfombra25(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 1;
    eventoAlfombra.idEvento = 25;
    eventoAlfombra.activo = 0;
    char texto[] = { "INERTE 2" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionAlfombra26(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 1;
    eventoAlfombra.idProgreso = 1;
    eventoAlfombra.idEvento = 26;
    eventoAlfombra.activo = 1;
    char texto[] = { "Has encontrado algo mas que simples pilas, o hay algo mas oscuro aguardando su despertar?" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
///FIN ÁRBOL ALFOMBRA
///INICIO ÁRBOL CAJON
void inicializarEventosHabitacionCajon(stEscenario *escenario) {
    printf("\n\MOSTRANDO INICIO DEBUG CAJON \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///LA REFERENCIA APUNTA A LA RAIZ

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol); ///BORRAR

    ///RAIZ CAJON
    stArbol inerteCajon0;
    nuevoSubArbol = &inerteCajon0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajon30(nuevoSubArbol); ///Inerte

    printf("\n RAIZ ALFOMBRA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = referenciaSubArbol->izquierda; ///Alfombra
    referenciaSubArbol->izquierda = nuevoSubArbol;///Cajon
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 Inerte
    stArbol inerteCajon1;
    nuevoSubArbol = &inerteCajon1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajon31(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ DEL ARBOL AL SUBARBOL NIVEL 1 DERECHA Cajon
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 1 Cajon

    ///PROGRESO 0 Activo (Desencadena eventos)
    stArbol activoCajon0;
    nuevoSubArbol = &activoCajon0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajonProgreso0(nuevoSubArbol);

    printf("\n PROGRESO 0 ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 IZQUIERDA Cajon

    ///PROGRESO 1 Inerte
    stArbol inerteCajon2;
    nuevoSubArbol = &inerteCajon2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajon34(nuevoSubArbol); ///inerte    progreso 1

    printf("\n PROGRESO 1 INERTE \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA Cajon
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 Cajon

    ///PROGRESO 1 Activo (Desencadena eventos)
    stArbol activoCajon1;
    nuevoSubArbol = &activoCajon1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajonProgreso1(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 2 Inerte
    stArbol inerteCajon3;
    nuevoSubArbol = &inerteCajon3;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajon38(nuevoSubArbol); ///inerte    progreso 1

    printf("\n PROGRESO 2 INERTE \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA Cajon
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 Cajon

    ///PROGRESO 2 Activo (Desencadena eventos)

    stArbol activoCajon2;
    nuevoSubArbol = &activoCajon2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCajon39(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    printf("\n\MOSTRANDO FIN DEBUG CAJON \n");
}
void inicializarEventosHabitacionCajonProgreso0(stArbol *subArbol) {
    inicializarEventosHabitacionCajon32(subArbol);
    inicializarEventosHabitacionCajon33(subArbol);
}
void inicializarEventosHabitacionCajonProgreso1(stArbol *subArbol) {
    inicializarEventosHabitacionCajon35(subArbol);
    inicializarEventosHabitacionCajon36(subArbol);
    inicializarEventosHabitacionCajon37(subArbol);
}
void inicializarEventosHabitacionCajon30(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 30;
    eventoAlfombra.activo = 0;
    char texto[] = { "INERTE 0" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon31(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 31;
    eventoAlfombra.activo = 0;
    char texto[] = { "INERTE 1" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon32(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 32;
    eventoAlfombra.activo = 1;
    char texto[] = { "Parece que el cajon esta bloqueado con una combinacion numerica. Una punta de papel sobresale, sugiriendo que hay algo mas en su interior." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon33(stArbol *subArbol) {
    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 0;
    eventoAlfombra.idEvento = 33;
    eventoAlfombra.activo = 1;
    char texto[] = { "Tirando de la punta de la hoja. Logras sacarla sin haber abierto el cajon. En la nota se lee “Presta atencion a los detalles bajo tus pies”." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon34(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 1;
    eventoAlfombra.idEvento = 34;
    eventoAlfombra.activo = 0;
    char texto[] = { "INERTE 2" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon35(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 1;
    eventoAlfombra.idEvento = 35;
    eventoAlfombra.activo = 1;
    char texto[] = { "Ingresando la clave descubierta en el zapato, logras abrir el candado." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon36(stArbol *subArbol) {
    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 1;
    eventoAlfombra.idEvento = 36;
    eventoAlfombra.activo = 1;
    char texto[] = { "La combinacion desbloqueo el cajon, revelando una nota antigua. Parece ser la clave de algo mas." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon37(stArbol *subArbol) {
    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 1;
    eventoAlfombra.idEvento = 37;
    eventoAlfombra.activo = 1;
    char texto[] = { "Se revela una partitura que parece emanar una extrania energia." };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon38(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 2;
    eventoAlfombra.idEvento = 38;
    eventoAlfombra.activo = 0;
    char texto[] = { "" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
void inicializarEventosHabitacionCajon39(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoAlfombra;

    eventoAlfombra.idObjeto = 7;
    eventoAlfombra.idProgreso = 2;
    eventoAlfombra.idEvento = 39;
    eventoAlfombra.activo = 1;
    char texto[] = { "Te atreverias a liberar las fuerzas ocultas que aguardan en las sombras de esta composicion misteriosa?" };
    strcpy(eventoAlfombra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoAlfombra);
}
///FIN ÁRBOL CAJON
///INICIO ÁRBOL ZAPATOS
void inicializarEventosHabitacionZapatos(stEscenario *escenario) {
    printf("\n\MOSTRANDO INICIO DEBUG ZAPATOS \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario);

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol);

    ///RAIZ ZAPATOS
    stArbol inerteZapatos0;
    nuevoSubArbol = &inerteZapatos0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionZapatos40(nuevoSubArbol); ///Inerte

    printf("\n RAIZ ALFOMBRA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol->izquierda = nuevoSubArbol; ///ZAPATOS
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 Inerte
    stArbol inerteZapatos1;
    nuevoSubArbol = &inerteZapatos1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionZapatos41(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 Inerte

    stArbol inerteZapatos2;
    nuevoSubArbol = &inerteZapatos2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionZapatos42(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 Activo

    stArbol activoZapatos0;
    nuevoSubArbol = &activoZapatos0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionZapatosProgreso1(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 2 Inerte

    stArbol inerteZapatos3;
    nuevoSubArbol = &inerteZapatos2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionZapatos45(nuevoSubArbol); ///INERTE

    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 2 Activo

    stArbol activoZapatos1;
    nuevoSubArbol = &activoZapatos1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionZapatos46(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    printf("\n\MOSTRANDO FIN DEBUG ZAPATOS \n");
}
void inicializarEventosHabitacionZapatosProgreso1(stArbol *subArbol) {
    inicializarEventosHabitacionZapatos43(subArbol);
    inicializarEventosHabitacionZapatos44(subArbol);
}
void inicializarEventosHabitacionZapatos40(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 0;
    eventoZapatos.idEvento = 40;
    eventoZapatos.activo = 0;
    char texto[] = { "INERTE 0" };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
void inicializarEventosHabitacionZapatos41(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 0;
    eventoZapatos.idEvento = 41;
    eventoZapatos.activo = 0;
    char texto[] = { "INERTE 1" };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
void inicializarEventosHabitacionZapatos42(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 1;
    eventoZapatos.idEvento = 42;
    eventoZapatos.activo = 0;
    char texto[] = { "INERTE 2" };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
void inicializarEventosHabitacionZapatos43(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 1;
    eventoZapatos.idEvento = 43;
    eventoZapatos.activo = 1;
    char texto[] = { "Examinas unos zapatos y descubres una mancha fresca color rojo" };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
void inicializarEventosHabitacionZapatos44(stArbol *subArbol) {
    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 1;
    eventoZapatos.idEvento = 44;
    eventoZapatos.activo = 1;
    char texto[] = { "Revisando ambos zapatos te das cuenta de que uno de ellos tiene numeros escritos." };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
void inicializarEventosHabitacionZapatos45(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 2;
    eventoZapatos.idEvento = 45;
    eventoZapatos.activo = 0;
    char texto[] = { "" };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
void inicializarEventosHabitacionZapatos46(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoZapatos;

    eventoZapatos.idObjeto = 3;
    eventoZapatos.idProgreso = 2;
    eventoZapatos.idEvento = 46;
    eventoZapatos.activo = 0;
    char texto[] = { "Los numeros resultan ser: 666" };
    strcpy(eventoZapatos.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoZapatos);
}
///FIN ÁRBOL ZAPATOS
///INICIO ÁRBOL GUITARRA
void inicializarEventosHabitacionGuitarra(stEscenario *escenario) {
    printf("\n\MOSTRANDO INICIO DEBUG GUITARRA \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario);

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol);

    ///RAIZ GUITARRA
    stArbol inerteGuitarra0;
    nuevoSubArbol = &inerteGuitarra0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionGuitarra50(nuevoSubArbol); ///Inerte

    printf("\n RAIZ ALFOMBRA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol->izquierda = nuevoSubArbol; ///GUITARRA
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 Inerte
    stArbol inerteGuitarra1;
    nuevoSubArbol = &inerteGuitarra1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionGuitarra51(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 ACTIVO
    stArbol activoGuitarra1;
    nuevoSubArbol = &activoGuitarra1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionGuitarraProgreso1(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 2 Inerte

    stArbol inerteGuitarra2;
    nuevoSubArbol = &inerteGuitarra2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionGuitarra55(nuevoSubArbol); ///INERTE

    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 2 Activo

    stArbol activoGuitarra2;
    nuevoSubArbol = &activoGuitarra2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionGuitarra56(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    printf("\n\MOSTRANDO FIN DEBUG GUITARRA \n");
}
void inicializarEventosHabitacionGuitarraProgreso1(stArbol *subArbol) {
    inicializarEventosHabitacionGuitarra52(subArbol);
    inicializarEventosHabitacionGuitarra53(subArbol);
    inicializarEventosHabitacionGuitarra54(subArbol);
}
void inicializarEventosHabitacionGuitarra50(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 0;
    eventoGuitarra.idEvento = 50;
    eventoGuitarra.activo = 0;
    char texto[] = { "INERTE 0" };
    strcpy(eventoGuitarra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
void inicializarEventosHabitacionGuitarra51(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 1;
    eventoGuitarra.idEvento = 51;
    eventoGuitarra.activo = 0;
    char texto[] = { "INERTE 1" };
    strcpy(eventoGuitarra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
void inicializarEventosHabitacionGuitarra52(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 1;
    eventoGuitarra.idEvento = 52;
    eventoGuitarra.activo = 1;
    char texto[] = { "Al tocar la guitarra. Emite una melodia por si sola…" };
    strcpy(eventoGuitarra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
void inicializarEventosHabitacionGuitarra53(stArbol *subArbol) {
    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 1;
    eventoGuitarra.idEvento = 53;
    eventoGuitarra.activo = 0;
    char texto[] = { "Guitarra melodia" };
    strcpy(eventoGuitarra.texto, texto);

    eventoGuitarra.sonido = LoadSound("guitarra.mp3");

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
void inicializarEventosHabitacionGuitarra54(stArbol *subArbol) {
    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 1;
    eventoGuitarra.idEvento = 54;
    eventoGuitarra.activo = 1;
    char texto[] = { "Un compartimento secreto se revela, y en su interior hay una llave inusual." };
    strcpy(eventoGuitarra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
void inicializarEventosHabitacionGuitarra55(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 2;
    eventoGuitarra.idEvento = 55;
    eventoGuitarra.activo = 0;
    char texto[] = { "" };
    strcpy(eventoGuitarra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
void inicializarEventosHabitacionGuitarra56(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoGuitarra;

    eventoGuitarra.idObjeto = 4;
    eventoGuitarra.idProgreso = 2;
    eventoGuitarra.idEvento = 56;
    eventoGuitarra.activo = 1;
    char texto[] = { "La llave parece abrir una cerradura de la puerta." };
    strcpy(eventoGuitarra.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoGuitarra);
}
///FIN ÁRBOL GUITARRA
///INICIO ÁRBOL ARMARIO
void inicializarEventosHabitacionArmario(stEscenario *escenario) {
    printf("\INICIO ARBOL ARMARIO\n");

    printf("\n\MOSTRANDO INICIO DEBUG ARMARIO \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario);

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol);

    ///RAIZ ARMARIO
    stArbol inerteArmario0;
    nuevoSubArbol = &inerteArmario0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionArmario70(nuevoSubArbol); ///Inerte

    printf("\n RAIZ ARMARIO \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol = referenciaSubArbol->izquierda; ///GUITARRA
    referenciaSubArbol->izquierda = nuevoSubArbol; ///ARMARIO
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 Inerte
    stArbol inerteArmario1;
    nuevoSubArbol = &inerteArmario1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionArmario71(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 ACTIVO
    stArbol activoArmario0;
    nuevoSubArbol = &activoArmario0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionArmarioProgreso0(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 1 Inerte
    stArbol inerteArmario2;
    nuevoSubArbol = &inerteArmario2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionArmario74(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 INERTE \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 ACTIVO
    stArbol activoArmario1;
    nuevoSubArbol = &activoArmario1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionArmarioProgreso1(nuevoSubArbol);
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    printf("\nFIN ARBOL ARMARIO\n");
}
void inicializarEventosHabitacionArmarioProgreso0(stArbol *subArbol) {
    inicializarEventosHabitacionArmario72(subArbol);
    inicializarEventosHabitacionArmario73(subArbol);
}
void inicializarEventosHabitacionArmarioProgreso1(stArbol *subArbol) {
    inicializarEventosHabitacionArmario75(subArbol);
    inicializarEventosHabitacionArmario76(subArbol);
}
void inicializarEventosHabitacionArmario70(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 0;
    eventoArmario.idEvento = 70;
    eventoArmario.activo = 0;
    char texto[] = { "INERTE 0" };
    strcpy(eventoArmario.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
void inicializarEventosHabitacionArmario71(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 0;
    eventoArmario.idEvento = 71;
    eventoArmario.activo = 0;
    char texto[] = { "INERTE 1" };
    strcpy(eventoArmario.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
void inicializarEventosHabitacionArmario72(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 0;
    eventoArmario.idEvento = 72;
    eventoArmario.activo = 1;
    char texto[] = { "Abriendo el espeluznante armario, se puede notar una presencia oscura." };
    strcpy(eventoArmario.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
void inicializarEventosHabitacionArmario73(stArbol *subArbol) {
    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 0;
    eventoArmario.idEvento = 73;
    eventoArmario.activo = 1;
    char texto[] = { "Sonido Aterrador" };
    strcpy(eventoArmario.texto, texto);

    eventoArmario.sonido = LoadSound("gritoAterrador.mp3");

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
void inicializarEventosHabitacionArmario74(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 1;
    eventoArmario.idEvento = 74;
    eventoArmario.activo = 0;
    char texto[] = { "INERTE 2" };
    strcpy(eventoArmario.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
void inicializarEventosHabitacionArmario75(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 1;
    eventoArmario.idEvento = 75;
    eventoArmario.activo = 1;
    char texto[] = { "Vez una llave en el fondo del armario y la agarras." };
    strcpy(eventoArmario.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
void inicializarEventosHabitacionArmario76(stArbol *subArbol) { ///INERTE
    stEvento eventoArmario;

    eventoArmario.idObjeto = 9;
    eventoArmario.idProgreso = 1;
    eventoArmario.idEvento = 76;
    eventoArmario.activo = 1;
    char texto[] = { "Parece desbloquear un candado de la puerta de la habitacion." };
    strcpy(eventoArmario.texto, texto);

    agregarNodoEnFila(subArbol->fila, eventoArmario);
}
///FIN ÁRBOL ARMARIO
///COMIENZO ARBOL ESPEJO
void inicializarEventosHabitacionEspejo(stEscenario *escenario) {
    printf("\INICIO ARBOL ESPEJO\n");

    printf("\n\MOSTRANDO INICIO DEBUG ESPEJO \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario);

    printf("\n PRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol);

    ///RAIZ ESPEJO
    stArbol inerte0;
    nuevoSubArbol = &inerte0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionEspejo60(nuevoSubArbol); ///Inerte

    printf("\n RAIZ ESPEJO \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol = referenciaSubArbol->izquierda; ///GUITARRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ARMARIO
    referenciaSubArbol->izquierda = nuevoSubArbol; ///ESPEJO
    referenciaSubArbol = nuevoSubArbol;

   ///PROGRESO 0 Inerte
    stArbol inerte1;
    nuevoSubArbol = &inerte1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionEspejo61(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 ESPEJO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 ACTIVO
    stArbol activo0;
    nuevoSubArbol = &activo0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionEspejo62(nuevoSubArbol);

    printf("\n PROGRESO 0 ESPEJO ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;
}
void inicializarEventosHabitacionEspejo60(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ESPEJO;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 60;
    nuevoEvento.activo = 0;
    char texto[] = { "test espejo interte 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionEspejo61(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ESPEJO;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 61;
    nuevoEvento.activo = 0;
    char texto[] = { "test espejo interte 1" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionEspejo62(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ESPEJO;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 62;
    nuevoEvento.activo = 1;
    char texto[] = { "test espejo activo 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
///FIN ARBOL ESPEJO
///COMIENZO ARBOL VENTANA
void inicializarEventosHabitacionVentana(stEscenario *escenario) {
    printf("\INICIO ARBOL VENTANA\n");

    printf("\n\MOSTRANDO INICIO DEBUG VENTANA \n");

    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario);

    printf("\n PRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol);

    ///RAIZ VENTANA
    stArbol inerteVentana0;
    nuevoSubArbol = &inerteVentana0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionVentana80(nuevoSubArbol); ///Inerte

    printf("\n RAIZ VENTANA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol = referenciaSubArbol->izquierda; ///GUITARRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ARMARIO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ESPEJO
    referenciaSubArbol->izquierda = nuevoSubArbol; ///VENTANA
    referenciaSubArbol = nuevoSubArbol;

   ///PROGRESO 0 Inerte
    stArbol inerteVentana1;
    nuevoSubArbol = &inerteVentana1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionVentana81(nuevoSubArbol); ///INERTE

    printf("\n PROGRESO 0 VENTANA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 ACTIVO
    stArbol activoVentana0;
    nuevoSubArbol = &activoVentana0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionVentana82(nuevoSubArbol);

    printf("\n PROGRESO 0 VENTANA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;
}
void inicializarEventosHabitacionVentana80(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = VENTANA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 80;
    nuevoEvento.activo = 0;
    char texto[] = { "test ventana inerte 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionVentana81(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = VENTANA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 81;
    nuevoEvento.activo = 0;
    char texto[] = { "test ventana inerte 1" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionVentana82(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = VENTANA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 82;
    nuevoEvento.activo = 1;
    char texto[] = { "test ventana activo 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
///FIN ARBOL VENTANA
///INICIO ARBOL CAMA
void inicializarEventosHabitacionCama(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///MOVIENDO LA REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///MUNIECA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol = referenciaSubArbol->izquierda; ///GUITARRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ARMARIO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ESPEJO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///VENTANA

    ///RAIZ CAMA
    stArbol inerte0;
    nuevoSubArbol = &inerte0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCama50(nuevoSubArbol);

    printf("\n RAIZ CAMA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///CAMA
    referenciaSubArbol = nuevoSubArbol;

   ///PROGRESO 0 INERTE
    stArbol inerte1;
    nuevoSubArbol = &inerte1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCama51(nuevoSubArbol);

    printf("\n PROGRESO 0 CAMA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 ACTIVO
    stArbol activo0;
    nuevoSubArbol = &activo0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCamaProgreso0(nuevoSubArbol);

    printf("\n PROGRESO 0 CAMA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 1 INERTE
    stArbol inerte2;
    nuevoSubArbol = &inerte2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCama54(nuevoSubArbol);

    printf("\n PROGRESO 1 CAMA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 ACTIVO
    stArbol activo1;
    nuevoSubArbol = &activo1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCama55(nuevoSubArbol);

    printf("\n PROGRESO 1 CAMA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 2 INERTE
    stArbol inerte3;
    nuevoSubArbol = &inerte3;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCama56(nuevoSubArbol);

    printf("\n PROGRESO 2 CAMA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 2 ACTIVO
    stArbol activo2;
    nuevoSubArbol = &activo2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionCama57(nuevoSubArbol);

    printf("\n PROGRESO 2 CAMA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;
}
void inicializarEventosHabitacionCamaProgreso0(stArbol *subArbol) {
    inicializarEventosHabitacionCama52(subArbol);
    inicializarEventosHabitacionCama53(subArbol);
}
void inicializarEventosHabitacionCama50(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 50;
    nuevoEvento.activo = 0;
    char texto[] = { "test cama inerte 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama51(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 51;
    nuevoEvento.activo = 0;
    char texto[] = { "test cama inerte 1" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama52(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 52;
    nuevoEvento.activo = 1;
    char texto[] = { "Esta cama guarda los inquietantes recuerdos de quienes descansaron aquí, cada sueño alberga sus propios secretos de terror." };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama53(stArbol *subArbol) {
    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 53;
    nuevoEvento.activo = 1;
    char texto[] = { "Los susurros espeluznantes flotan en la almohada, como ecos de almas atrapadas." };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama54(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 1;
    nuevoEvento.idEvento = 54;
    nuevoEvento.activo = 0;
    char texto[] = { "test cama inerte 3" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama55(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 1;
    nuevoEvento.idEvento = 55;
    nuevoEvento.activo = 1;
    char texto[] = { "Voz: Has cometido un grave error al dejarme salir" };
    strcpy(nuevoEvento.texto, texto);

    nuevoEvento.sonido = LoadSound("HasCometidoUnGraveErrorAlDejarmeSalir.mp3"); ///AUDIO

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama56(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 2;
    nuevoEvento.idEvento = 56;
    nuevoEvento.activo = 0;
    char texto[] = { "test cama inerte 4" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionCama57(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = CAMA;
    nuevoEvento.idProgreso = 2;
    nuevoEvento.idEvento = 57;
    nuevoEvento.activo = 1;
    char texto[] = { "Has encontrado una llave vieja y oxidada debajo de las sabanas de la cama. Que abrira?" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
///FIN ARBOL CAMA
///INICIO ARBOL ALMOHADA
void inicializarEventosHabitacionAlmohada(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///MOVIENDO LA REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///MUNIECA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol = referenciaSubArbol->izquierda; ///GUITARRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ARMARIO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ESPEJO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///VENTANA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAMA

    ///RAIZ ALMOHADA
    stArbol inerte0;
    nuevoSubArbol = &inerte0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlmohada90(nuevoSubArbol);

    printf("\n RAIZ ALMOHADA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///ALMOHADA
    referenciaSubArbol = nuevoSubArbol;

   ///PROGRESO 0 INERTE
    stArbol inerte1;
    nuevoSubArbol = &inerte1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlmohada91(nuevoSubArbol);

    printf("\n PROGRESO 0 ALMOHADA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 ACTIVO
    stArbol activo0;
    nuevoSubArbol = &activo0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlmohadaProgreso0(nuevoSubArbol);

    printf("\n PROGRESO 0 ALMOHADA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 1 INERTE
    stArbol inerte2;
    nuevoSubArbol = &inerte2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlmohada98(nuevoSubArbol);

    printf("\n PROGRESO 1 ALMOHADA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 ACTIVO
    stArbol activo1;
    nuevoSubArbol = &activo1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionAlmohada99(nuevoSubArbol);

    printf("\n PROGRESO 1 ALMOHADA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;
}
void inicializarEventosHabitacionAlmohadaProgreso0 (stArbol *subArbol) {
    inicializarEventosHabitacionAlmohada92(subArbol);
    inicializarEventosHabitacionAlmohada93(subArbol);
    inicializarEventosHabitacionAlmohada94(subArbol);
    inicializarEventosHabitacionAlmohada95(subArbol);
    inicializarEventosHabitacionAlmohada96(subArbol);
    inicializarEventosHabitacionAlmohada97(subArbol);
}
void inicializarEventosHabitacionAlmohada90(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 90;
    nuevoEvento.activo = 0;
    char texto[] = { "test almohada inerte 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada91(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 91;
    nuevoEvento.activo = 0;
    char texto[] = { "test almohada inerte 1" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada92(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 92;
    nuevoEvento.activo = 1;
    char texto[] = { "Se puede escuchar como una voz bajita sale de la almohada" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada93(stArbol *subArbol) {
    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 93;
    nuevoEvento.activo = 1;
    char texto[] = { "Voz – Acercate para escucharme" };
    strcpy(nuevoEvento.texto, texto);

    nuevoEvento.sonido = LoadSound("AcercateParaEscucharme.mp3"); ///AUDIO

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada94(stArbol *subArbol) {
    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 94;
    nuevoEvento.activo = 1;
    char texto[] = { "Has encontrado un dedo arrancado de un humano en descomposición debajo de la almohada" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada95(stArbol *subArbol) {
    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 95;
    nuevoEvento.activo = 1;
    char texto[] = { "Voz: Debes encontrar restos humanos diseccionados en la habitacion." };
    strcpy(nuevoEvento.texto, texto);

    nuevoEvento.sonido = LoadSound("DebesEncontrarRestosHumanos.mp3"); ///AUDIO

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada96(stArbol *subArbol) {
    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 96;
    nuevoEvento.activo = 1;
    char texto[] = { "Voz: Pero rápido ! antes de que los otros espíritus me escuchen." };
    strcpy(nuevoEvento.texto, texto);

    nuevoEvento.sonido = LoadSound("PeroRapidoAntesDeQueLosOtrosEscuchen.mp3"); ///AUDIO

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada97(stArbol *subArbol) {
    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 97;
    nuevoEvento.activo = 1;
    char texto[] = { "A simple vista se puede notar como la luna llena resalta en la habitación." };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada98(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 1;
    nuevoEvento.idEvento = 98;
    nuevoEvento.activo = 0;
    char texto[] = { "test almohada inerte 2" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionAlmohada99(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = ALMOHADA;
    nuevoEvento.idProgreso = 1;
    nuevoEvento.idEvento = 99;
    nuevoEvento.activo = 0;
    char texto[] = { "Ya encontraste el trozo humano debajo de la almohada, sigue revisando la habitación…" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
///FIN ARBOL ALMOHADA
void inicializarEventosHabitacionPuerta(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///MOVIENDO LA REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///MUNIECA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALFOMBRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAJON
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ZAPATOS
    referenciaSubArbol = referenciaSubArbol->izquierda; ///GUITARRA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ARMARIO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ESPEJO
    referenciaSubArbol = referenciaSubArbol->izquierda; ///VENTANA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///CAMA
    referenciaSubArbol = referenciaSubArbol->izquierda; ///ALMOHADA

    ///RAIZ PUERTA
    stArbol inerte0;
    nuevoSubArbol = &inerte0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta100(nuevoSubArbol);

    printf("\n RAIZ PUERTA \n"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///PUERTA
    referenciaSubArbol = nuevoSubArbol;

   ///PROGRESO 0 INERTE
    stArbol inerte1;
    nuevoSubArbol = &inerte1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta101(nuevoSubArbol);

    printf("\n PROGRESO 0 PUERTA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 0 ACTIVO
    stArbol activo0;
    nuevoSubArbol = &activo0;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta102(nuevoSubArbol);

    printf("\n PROGRESO 0 PUERTA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 1 INERTE
    stArbol inerte2;
    nuevoSubArbol = &inerte2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta103(nuevoSubArbol);

    printf("\n PROGRESO 1 PUERTA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 1 ACTIVO
    stArbol activo1;
    nuevoSubArbol = &activo1;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta104(nuevoSubArbol);

    printf("\n PROGRESO 1 PUERTA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 2 INERTE
    stArbol inerte3;
    nuevoSubArbol = &inerte3;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta105(nuevoSubArbol);

    printf("\n PROGRESO 2 PUERTA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 2 ACTIVO
    stArbol activo2;
    nuevoSubArbol = &activo2;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta106(nuevoSubArbol);

    printf("\n PROGRESO 2 PUERTA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 3 INERTE
    stArbol inerte4;
    nuevoSubArbol = &inerte4;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta107(nuevoSubArbol);

    printf("\n PROGRESO 3 PUERTA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 4 ACTIVO
    stArbol activo3;
    nuevoSubArbol = &activo3;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta108(nuevoSubArbol);

    printf("\n PROGRESO 3 PUERTA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;

    ///PROGRESO 4 INERTE
    stArbol inerte5;
    nuevoSubArbol = &inerte5;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta109(nuevoSubArbol);

    printf("\n PROGRESO 4 PUERTA \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol;
    referenciaSubArbol = nuevoSubArbol;

    ///PROGRESO 4 ACTIVO
    stArbol activo4;
    nuevoSubArbol = &activo4;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionPuerta110(nuevoSubArbol);

    printf("\n PROGRESO 4 PUERTA ACTIVO \n");///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol;
}
void inicializarEventosHabitacionPuerta100(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 100;
    nuevoEvento.activo = 0;
    char texto[] = { "test puerta inerte 0" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta101(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 101;
    nuevoEvento.activo = 0;
    char texto[] = { "test puerta inerte 1" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta102(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 0;
    nuevoEvento.idEvento = 102;
    nuevoEvento.activo = 1;
    char texto[] = { "La puerta parece estar cerrada con tres cerraduras diferentes, en algún lugar de la habitación deben estar las llaves que la abren…" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta103(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 1;
    nuevoEvento.idEvento = 103;
    nuevoEvento.activo = 0;
    char texto[] = { "test puerta inerte 2" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta104(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 1;
    nuevoEvento.idEvento = 104;
    nuevoEvento.activo = 1;
    char texto[] = { "Abriste una de las cerraduras con la llave, pero aun quedan dos mas y pareciera que cada vez hay menos tiempo…" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta105(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 2;
    nuevoEvento.idEvento = 105;
    nuevoEvento.activo = 0;
    char texto[] = { "test puerta inerte 3" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta106(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 2;
    nuevoEvento.idEvento = 106;
    nuevoEvento.activo = 1;
    char texto[] = { "Lograste abrir otra de las cerraduras, solo queda una mas, puede que logres salir antes que lleguen…" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta107(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 3;
    nuevoEvento.idEvento = 107;
    nuevoEvento.activo = 0;
    char texto[] = { "test puerta inerte 4" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta108(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 3;
    nuevoEvento.idEvento = 108;
    nuevoEvento.activo = 1;
    char texto[] = { "Has logrado escapar de la habitación con vida!" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta109(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 4;
    nuevoEvento.idEvento = 109;
    nuevoEvento.activo = 0;
    char texto[] = { "test puerta inerte 5" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarEventosHabitacionPuerta110(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nuevoEvento;

    nuevoEvento.idObjeto = PUERTA;
    nuevoEvento.idProgreso = 4;
    nuevoEvento.idEvento = 110;
    nuevoEvento.activo = 1;
    char texto[] = { "GANASTE!" };
    strcpy(nuevoEvento.texto, texto);

    agregarNodoEnFila(subArbol->fila, nuevoEvento);
}
void inicializarObjetosHabitacion(stEscenario *escenario) {
    inicializarObjetoMunieca(escenario);
    inicializarObjetoAlfombra(escenario);
    inicializarObjetoEspejo(escenario);
    inicializarObjetoZapatos(escenario);
    inicializarObjetoGuitarra(escenario);
    inicializarObjetoCama(escenario);
    inicializarObjetoVentana(escenario);
    inicializarObjetoFondoVentana(escenario);
    inicializarObjetoCajon(escenario);
    inicializarObjetoAlmohada(escenario);
    inicializarObjetoArmario(escenario);
    inicializarObjetoPuerta(escenario);
    inicializarObjetoOreja(escenario);
    inicializarObjetoFondoEspejo(escenario);
}
void inicializarObjetoMunieca(stEscenario *escenario) {
    escenario[HABITACION].objetos[MUNIECA].existe = 1; //Muñeca WHITE
    escenario[HABITACION].objetos[MUNIECA].hitbox.x = 1398;
    escenario[HABITACION].objetos[MUNIECA].hitbox.y = 420; //el eje y esta invertido
    escenario[HABITACION].objetos[MUNIECA].hitbox.width = 61; //el eje y esta invertido
    escenario[HABITACION].objetos[MUNIECA].hitbox.height = 190; //el eje y esta invertido
}
void inicializarObjetoAlfombra(stEscenario *escenario) {
    escenario[HABITACION].objetos[ALFOMBRA].existe = 1;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.x = 1272;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.y = 866;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.width = 80;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.height = 77;
}
void inicializarObjetoEspejo(stEscenario *escenario) {
    escenario[HABITACION].objetos[ESPEJO].existe = 1;
    escenario[HABITACION].objetos[ESPEJO].hitbox.x = 567;
    escenario[HABITACION].objetos[ESPEJO].hitbox.y = 149;
    escenario[HABITACION].objetos[ESPEJO].hitbox.width = 115;
    escenario[HABITACION].objetos[ESPEJO].hitbox.height = 430;
}
void inicializarObjetoZapatos(stEscenario *escenario) {
    escenario[HABITACION].objetos[ZAPATOS].existe = 1;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.x = 1126;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.y = 855;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.width = 64;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.height = 100;
}
void inicializarObjetoGuitarra(stEscenario *escenario) {
    escenario[HABITACION].objetos[GUITARRA].existe = 1;
    escenario[HABITACION].objetos[GUITARRA].hitbox.x = 854;
    escenario[HABITACION].objetos[GUITARRA].hitbox.y = 643;
    escenario[HABITACION].objetos[GUITARRA].hitbox.width = 63;
    escenario[HABITACION].objetos[GUITARRA].hitbox.height = 91;
}
void inicializarObjetoCama(stEscenario *escenario) {
    escenario[HABITACION].objetos[CAMA].existe = 1;
    escenario[HABITACION].objetos[CAMA].hitbox.x = 830;
    escenario[HABITACION].objetos[CAMA].hitbox.y = 490;
    escenario[HABITACION].objetos[CAMA].hitbox.width = 194;
    escenario[HABITACION].objetos[CAMA].hitbox.height = 87;
}
void inicializarObjetoVentana(stEscenario *escenario) {
    escenario[HABITACION].objetos[VENTANA].existe = 1;
    escenario[HABITACION].objetos[VENTANA].hitbox.x = 944;
    escenario[HABITACION].objetos[VENTANA].hitbox.y = 185;
    escenario[HABITACION].objetos[VENTANA].hitbox.width = 84;
    escenario[HABITACION].objetos[VENTANA].hitbox.height = 214;
}
void inicializarObjetoFondoVentana(stEscenario *escenario) {
    inicializarObjetoFondoVentana1(escenario);
    inicializarObjetoFondoVentana2(escenario);
    inicializarObjetoFondoVentana3(escenario);
    inicializarObjetoFondoVentana4(escenario);
}
void inicializarObjetoFondoVentana1(stEscenario *escenario) {
    escenario[HABITACION].objetos[FONDOVENTANA1].existe = 1;
    escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.x = 501;
    escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.y = 14;
    escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.width = 211;
    escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.height = 1014;
}
void inicializarObjetoFondoVentana2(stEscenario *escenario) {
    escenario[HABITACION].objetos[FONDOVENTANA2].existe = 1;
    escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.x = 1311;
    escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.y = 14;
    escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.width = 219;
    escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.height = 1015;
}
void inicializarObjetoFondoVentana3(stEscenario *escenario) {
    escenario[HABITACION].objetos[FONDOVENTANA3].existe = 1;
    escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.x = 712;
    escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.y = 14;
    escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.width = 600;
    escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.height = 67;
}
void inicializarObjetoFondoVentana4(stEscenario *escenario) {
    escenario[HABITACION].objetos[FONDOVENTANA4].existe = 1;
    escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.x = 713;
    escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.y = 973;
    escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.width = 600;
    escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.height = 56;
}
void inicializarObjetoCajon(stEscenario *escenario) {
    escenario[HABITACION].objetos[CAJON].existe = 1;
    escenario[HABITACION].objetos[CAJON].hitbox.x = 626;
    escenario[HABITACION].objetos[CAJON].hitbox.y = 653;
    escenario[HABITACION].objetos[CAJON].hitbox.width = 47;
    escenario[HABITACION].objetos[CAJON].hitbox.height = 46;
}
void inicializarObjetoAlmohada(stEscenario *escenario) {
    escenario[HABITACION].objetos[ALMOHADA].existe = 1;
    escenario[HABITACION].objetos[ALMOHADA].hitbox.x = 880;
    escenario[HABITACION].objetos[ALMOHADA].hitbox.y = 428;
    escenario[HABITACION].objetos[ALMOHADA].hitbox.width = 164;
    escenario[HABITACION].objetos[ALMOHADA].hitbox.height = 59;
}
void inicializarObjetoArmario(stEscenario *escenario) {
    escenario[HABITACION].objetos[ARMARIO].existe = 1;
    escenario[HABITACION].objetos[ARMARIO].hitbox.x = 722;
    escenario[HABITACION].objetos[ARMARIO].hitbox.y = 200;
    escenario[HABITACION].objetos[ARMARIO].hitbox.width = 59;
    escenario[HABITACION].objetos[ARMARIO].hitbox.height = 345;
}
void inicializarObjetoPuerta(stEscenario *escenario) {
    escenario[HABITACION].objetos[CANDADOPUERTA].existe = 1;
    escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.x = 1240;
    escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.y = 170;
    escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.width = 160;
    escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.height = 415;
}
void inicializarObjetoOreja(stEscenario *escenario) {
    escenario[HABITACION].objetos[OREJA].existe = 1;
    escenario[HABITACION].objetos[OREJA].hitbox.x = 1198;
    escenario[HABITACION].objetos[OREJA].hitbox.y = 815;
    escenario[HABITACION].objetos[OREJA].hitbox.width = 31;
    escenario[HABITACION].objetos[OREJA].hitbox.height = 57;
}
void inicializarObjetoFondoEspejo(stEscenario *escenario) {
    inicializarObjetoFondoEspejo1(escenario);
    inicializarObjetoFondoEspejo2(escenario);
}
void inicializarObjetoFondoEspejo1(stEscenario *escenario) {
    escenario[HABITACION].objetos[FONDOESPEJO1].existe = 1;
    escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.x = 500;
    escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.y = 13;
    escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.width = 173;
    escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.height = 1016;
}
void inicializarObjetoFondoEspejo2(stEscenario *escenario) {
    escenario[HABITACION].objetos[FONDOESPEJO2].existe = 1;
    escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.x = 1350;
    escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.y = 13;
    escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.width = 173;
    escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.height = 1016;
}
/////COCINA
//void inicializarCocina(stEscenario *escenario) {
//    escenario[COCINA].fondoEstatico = LoadTexture("cocina.png");
//    ///FUNCIONES A DESARROLLAR
//    //inicializarEventosCocina(escenario);
//    //inicializarArbolCocina(escenario);
//    inicializarObjetosCocina(escenario);
//}
//void inicializarEventosCocina(stEscenario *escenario) {  /// FUNCION A REALIZAR CUANDO TENGAMOS LOS PUZZLES
////    inicializarEventosHabitacionCuadro(escenario);
////    inicializarEventosHabitacionTacho(escenario);
////    inicializarEventosHabitacionHeladera(escenario);
////    inicializarEventosHabitacionCuchillo(escenario);
////    inicializarEventosHabitacionPared(escenario);
////    inicializarEventosHabitacionMesada(escenario);
////    inicializarEventosHabitacionPuertaCocina(escenario);
//}
//void inicializarObjetosCocina(stEscenario *escenario) {
//    inicializarObjetoCuadro(escenario);
//    inicializarObjetoTacho(escenario);
//    inicializarObjetoHeladera(escenario);
//    inicializarObjetoCuchillo(escenario);
//    inicializarObjetoPared(escenario);
//    inicializarObjetoMesada(escenario);
//    //inicializarObjetoPuerta(escenario);
//}
//void inicializarObjetoCuadro(stEscenario *escenario) {
//    escenario[COCINA].objetos[CUADRO].existe = 1;
//    escenario[COCINA].objetos[CUADRO].hitbox.x = 542;
//    escenario[COCINA].objetos[CUADRO].hitbox.y = 40;
//    escenario[COCINA].objetos[CUADRO].hitbox.width = 105;
//    escenario[COCINA].objetos[CUADRO].hitbox.height = 228;
//}
//void inicializarObjetoTacho(stEscenario *escenario) {
//    escenario[COCINA].objetos[TACHO].existe = 1;
//    escenario[COCINA].objetos[TACHO].hitbox.x = 849;
//    escenario[COCINA].objetos[TACHO].hitbox.y = 521;
//    escenario[COCINA].objetos[TACHO].hitbox.width = 100;
//    escenario[COCINA].objetos[TACHO].hitbox.height = 177;
//}
//void inicializarObjetoHeladera(stEscenario *escenario) {
//    escenario[COCINA].objetos[HELADERA].existe = 1;
//    escenario[COCINA].objetos[HELADERA].hitbox.x = 811;
//    escenario[COCINA].objetos[HELADERA].hitbox.y = 205;
//    escenario[COCINA].objetos[HELADERA].hitbox.width = 119;
//    escenario[COCINA].objetos[HELADERA].hitbox.height = 240;
//}
//void inicializarObjetoCuchillo(stEscenario *escenario) {
//    escenario[COCINA].objetos[CUCHILLO].existe = 1;
//    escenario[COCINA].objetos[CUCHILLO].hitbox.x = 809;
//    escenario[COCINA].objetos[CUCHILLO].hitbox.y = 760;
//    escenario[COCINA].objetos[CUCHILLO].hitbox.width = 64;
//    escenario[COCINA].objetos[CUCHILLO].hitbox.height = 150;
//}
//void inicializarObjetoPared(stEscenario *escenario) {
//    escenario[COCINA].objetos[PARED].existe = 1;
//    escenario[COCINA].objetos[PARED].hitbox.x = 1200;
//    escenario[COCINA].objetos[PARED].hitbox.y = 340;
//    escenario[COCINA].objetos[PARED].hitbox.width = 224;
//    escenario[COCINA].objetos[PARED].hitbox.height = 47;
//}
//void inicializarObjetoMesada(stEscenario *escenario) {
//    escenario[COCINA].objetos[MESADA].existe = 1;
//    escenario[COCINA].objetos[MESADA].hitbox.x = 553;
//    escenario[COCINA].objetos[MESADA].hitbox.y = 464;
//    escenario[COCINA].objetos[MESADA].hitbox.width = 261;
//    escenario[COCINA].objetos[MESADA].hitbox.height = 197;
//}
//
////void inicializarObjetoPuerta(stEscenario *escenario) {
////
////    escenario[COCINA].objetos[PUERTA].existe = 1;
////    escenario[COCINA].objetos[PUERTA].hitbox.x = 998;
////    escenario[COCINA].objetos[PUERTA].hitbox.y = 135;
////    escenario[COCINA].objetos[PUERTA].hitbox.width = 162;
////    escenario[COCINA].objetos[PUERTA].hitbox.height = 402;
////}
//
/////PATIO
//void inicializarJardin(stEscenario *escenario) {
//    escenario[JARDIN].fondoEstatico = LoadTexture("jardin.png");
//    ///FUNCIONES A DESARROLLARã
//    //inicializarEventosJardin(escenario);
//    //inicializarArbolJardin(escenario);
//    //inicializarObjetosJardin(escenario);
//}
///MENU
void inicializarMenu(stEscenario *escenario) {
    escenario[MENU].fondoEstatico = LoadTexture("menu.png");
    inicializarBotonesMenu(escenario);
}
void inicializarBotonesMenu(stEscenario *escenario) {
    inicializarBotonPlay(escenario);
    inicializarBotonLoad(escenario);
    inicializarBotonScore(escenario);
    inicializarBotonSave(escenario);
    inicializarBotonCredits(escenario);
}
void inicializarBotonPlay(stEscenario *escenario) {
    escenario[MENU].botones[PLAY] = LoadTexture("play.png");
    escenario[MENU].objetos[PLAY].existe = 1;
    escenario[MENU].objetos[PLAY].hitbox.x = 881;
    escenario[MENU].objetos[PLAY].hitbox.y = 820;
    escenario[MENU].objetos[PLAY].hitbox.width = 262;
    escenario[MENU].objetos[PLAY].hitbox.height = 76;
}
void inicializarBotonLoad(stEscenario *escenario) {
    escenario[MENU].botones[BOTONLOAD] = LoadTexture("boton.png");
    escenario[MENU].objetos[BOTONLOAD].existe = 1;
    escenario[MENU].objetos[BOTONLOAD].hitbox.x = 550;
    escenario[MENU].objetos[BOTONLOAD].hitbox.y = 702;
    escenario[MENU].objetos[BOTONLOAD].hitbox.width = 210;
    escenario[MENU].objetos[BOTONLOAD].hitbox.height = 49;
}
void inicializarBotonScore(stEscenario *escenario) {
    escenario[MENU].botones[BOTONSCORE] = LoadTexture("boton.png");
    escenario[MENU].objetos[BOTONSCORE].existe = 1;
    escenario[MENU].objetos[BOTONSCORE].hitbox.x = 550;
    escenario[MENU].objetos[BOTONSCORE].hitbox.y = 802;
    escenario[MENU].objetos[BOTONSCORE].hitbox.width = 210;
    escenario[MENU].objetos[BOTONSCORE].hitbox.height = 49;
}
void inicializarBotonSave(stEscenario *escenario) {
    escenario[MENU].botones[BOTONSAVE] = LoadTexture("boton.png");
    escenario[MENU].objetos[BOTONSAVE].existe = 1;
    escenario[MENU].objetos[BOTONSAVE].hitbox.x = 1258;
    escenario[MENU].objetos[BOTONSAVE].hitbox.y = 702;
    escenario[MENU].objetos[BOTONSAVE].hitbox.width = 210;
    escenario[MENU].objetos[BOTONSAVE].hitbox.height = 49;
}
void inicializarBotonCredits(stEscenario *escenario) {
    escenario[MENU].botones[BOTONCREDITS] = LoadTexture("boton.png");
    escenario[MENU].objetos[BOTONCREDITS].existe = 1;
    escenario[MENU].objetos[BOTONCREDITS].hitbox.x = 1258;
    escenario[MENU].objetos[BOTONCREDITS].hitbox.y = 802;
    escenario[MENU].objetos[BOTONCREDITS].hitbox.width = 210;
    escenario[MENU].objetos[BOTONCREDITS].hitbox.height = 49;
}
///CREDITOS
void inicializarCreditos(stEscenario *escenario) {
    escenario[CREDITOS].fondoEstatico = LoadTexture("creditos.png");
    inicializarBotonesCreditos(escenario);
}
void inicializarBotonesCreditos(stEscenario *escenario) {
    inicializarBotonBack(escenario);
}
void inicializarBotonBack(stEscenario *escenario){
    escenario[CREDITOS].botones[BACK] = LoadTexture("boton.png");
    escenario[CREDITOS].objetos[BACK].existe = 1;
    escenario[CREDITOS].objetos[BACK].hitbox.x = 878;
    escenario[CREDITOS].objetos[BACK].hitbox.y = 950;
    escenario[CREDITOS].objetos[BACK].hitbox.width = 210;
    escenario[CREDITOS].objetos[BACK].hitbox.height = 53;
}
///GANASTE
void inicializarGanaste(stEscenario *escenario) {
    escenario[GANASTE].fondoEstatico = LoadTexture("ganaste.png");
    inicializarBotonSalirDeGanaste(escenario);
}
void inicializarBotonSalirDeGanaste(stEscenario *escenario){
    escenario[GANASTE].objetos[BOTONGANASTE].existe = 1;
    escenario[GANASTE].objetos[BOTONGANASTE].hitbox.x = 503;
    escenario[GANASTE].objetos[BOTONGANASTE].hitbox.y = 13;
    escenario[GANASTE].objetos[BOTONGANASTE].hitbox.width = 1021;
    escenario[GANASTE].objetos[BOTONGANASTE].hitbox.height = 1017;
}
///SAVE
void inicializarSave(stEscenario *escenario) {
    escenario[SAVE].fondoEstatico = LoadTexture("loadsave.png"); ///TOQUIE ESTO

}
void inicializarScore(stEscenario *escenario) {
    escenario[SCORE].fondoEstatico = LoadTexture("score.png"); ///TOQUE ESTO
    inicializarBotonBackScore(escenario);
}
void inicializarGameOver(stEscenario *escenario) {
    escenario[GAMEOVER].fondoEstatico = LoadTexture("score.png"); ///TOQUE ESTO
    inicializarBotonBackScore(escenario);
}
void inicializarBotonBackScore(stEscenario *escenario){
    escenario[SCORE].botones[BOTONSCOREBACK] = LoadTexture("botonScore.png");
    escenario[SCORE].objetos[BOTONSCOREBACK].existe = 1;
    escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.x = 925;
    escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.y = 897;
    escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.width = 216;
    escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.height = 53;
}
///ACTUALIZAR
void actualizar(stEscenario *escenario, stJugador *player, Debug *herramientaDebug) {
    if( player->escenario == HABITACION ) {
        actualizarInputHabitacion(escenario, player);
        actualizarArbolHabitacion(escenario, player);
        actualizarEventosHabitacion(escenario, player);
        actualizarScore(player);
    }
//    if( player->escenario == COCINA ) {
//        //actualizarInputCocina(escenario, player);
//        //actualizarArbolCocina(escenario, player, player->flags[player->idObjeto].idProgreso);
//        //actualizarEventosCocina(escenario, player);
//    }
    if( player->escenario == MENU ) {
        actualizarInputMenu(escenario, player);
    }

    if( player->escenario == CREDITOS ) {
        actualizarInputCreditos(escenario, player);
        actualizarPosicionCreditos(player);
    }
    if( player->escenario == GANASTE ) {   ///TOQUE ACA
        actualizarInputGanaste(escenario, player);
    }
    if( player->escenario == SCORE ) {   ///TOQUE ACA
        actualizarInputBackScore(escenario, player);
    }
    if( player->escenario == SAVE ) {
        actualizarInputSave(player);
    }
    if( herramientaDebug->mostrarHerramienta == 1 ) {
        actualizarInputDebug(herramientaDebug);
    }
    player->idObjeto = -1; ///DESCLICKEA LOS OBJETOS CUANDO TERMINA DE ACTUALIZAR
}
///PRUEBA TEST DE CLICK DE ALGUNOS INPUT (OBJETOS A INTERACTUAR)
void actualizarInputHabitacion(stEscenario *escenario, stJugador *player){
    if( player->textoActivo == 0 && player->subEscenarioActivo == 0) {
        if( seClickeaLaMunieca(escenario) )      interactuarConLaMunieca(player);
        if( seClickeaElEspejo(escenario) )       interactuarConElEspejo(player);
        if( seClickeaLaVentana(escenario) )      interactuarConLaVentana(player);
        if( seClickeaLaCama(escenario) )         interactuarConLaCama(player);
        if( seClickeaLaAlfombra(escenario) )     interactuarConLaAlfombra(player);
        if( seClickeaElCajon(escenario) )        interactuarConElCajon(player);
        if( seClickeaLaAlmohada(escenario) )     interactuarConLaAlmohada(player);
        if( seClickeaLaPuertaSalida(escenario) ) interactuarConLaPuerta(player);

        ///EDITANDO LAS COMPROBACIONES DEL FLAG NO TIENEN Q ESTAR ACA
        if( player->flags[CAJON].idEvento == 33 ) if( seClickeanLosZapatos(escenario) )   interactuarConLosZapatos(player); ///la comprobacion esta mal puede romper el programa, tiene que revisar el progreso, tampoco hay q hacer andar el hitbox de esta forma
        if( player->flags[CAJON].idEvento == 39 ) if( seClickeaGuitarra(escenario) )      interactuarConLaGuitarra(player); ///la comprobacion esta mal puede romper el programa, tiene que revisar el progreso, tampoco hay q hacer andar el hitbox de esta forma
        if( player->flags[MUNIECA].idEvento == 13 || player->flags[MUNIECA].idEvento == 14 || player->flags[MUNIECA].idEvento == 15 ) if( seClickeaArmario(escenario) )     interactuarConElArmario(player);
    }
    if( player->textoActivo == 1 ) {
        if( seClickeaDentroDelTexto(escenario) )interactuarDentroDelTexto(player);
        if( seClickeaFueraDelTexto(escenario) ) interactuarFueraDelTexto(player);
    }
    if( player->subEscenarioActivo == 1 ) { /// HAY QUE VER SI ESTA BIEN
        if( seClickeaElFondoVentana1(escenario) || seClickeaElFondoVentana2(escenario) || seClickeaElFondoVentana3(escenario) || seClickeaElFondoVentana4(escenario) )
            interactuarFueraDelSubEscenario(player);
        if( seClickeaLaOreja(escenario) ) interactuarConLaOreja(player);
    }
    if( player->subEscenarioActivo == 2 ) {
        ///del sub escenario espejo
        if( seClickeaElFondoEspejo1(escenario) || seClickeaElFondoEspejo2(escenario) )
            interactuarFueraDelSubEscenario(player);
    }
    if( IsKeyReleased(KEY_ENTER) )
        volverAlMenu(player);
}
void seClickea() {
    int esClickeada = 0;
}
void volverAlMenu(stJugador *player) {
    player->escenario = MENU;
}
int seClickeaLaMunieca(stEscenario *escenario) {
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[MUNIECA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElEspejo(stEscenario *escenario) { ///SubEscenario
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ESPEJO].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaLaVentana(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[VENTANA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaCama(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[CAMA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElCajon(stEscenario *escenario) { ///SubEscenario
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[CAJON].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeanLosZapatos(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ZAPATOS].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaGuitarra(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[GUITARRA].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaArmario(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ARMARIO].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaLaAlfombra(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ALFOMBRA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaDentroDelTexto(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[DENTROTEXTO].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaFueraDelTexto(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FUERATEXTO].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElFondoVentana1(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FONDOVENTANA1].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElFondoVentana2(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FONDOVENTANA2].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElFondoVentana3(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FONDOVENTANA3].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElFondoVentana4(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FONDOVENTANA4].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaOreja(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[OREJA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElFondoEspejo1(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FONDOESPEJO1].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElFondoEspejo2(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[FONDOESPEJO2].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaAlmohada(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ALMOHADA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaPuertaSalida(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[CANDADOPUERTA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
void interactuarConLaMunieca(stJugador *player) {
    player->idObjeto = MUNIECA;
    player->ultimoObjetoClickeado = MUNIECA;
    printf("\nMUNIECA CLICKEADO\n");
}
void interactuarConElEspejo(stJugador *player) {
    player->idObjeto = ESPEJO;
    player->ultimoObjetoClickeado = ESPEJO;
    printf("\nESPEJO CLICKEADO\n");
}
void interactuarConLaVentana(stJugador *player) {
    player->idObjeto = VENTANA;
    player->ultimoObjetoClickeado = VENTANA;
    printf("\nVENTANA CLICKEADA\n");
}
void interactuarConLaCama(stJugador *player) {
    player->idObjeto = CAMA;
    player->ultimoObjetoClickeado = CAMA;
    printf("\nCAMA CLICKEADA\n");
}
void interactuarConLaAlfombra(stJugador *player) {
    player->idObjeto = ALFOMBRA;
    player->ultimoObjetoClickeado = ALFOMBRA;
    printf("\nALFOMBRA CLICKEADA\n");
}
void interactuarConElCajon(stJugador *player) {
    player->idObjeto = CAJON;
    player->ultimoObjetoClickeado = CAJON;
    printf("\nCAJON CLICKEADO\n");
}
void interactuarConLosZapatos(stJugador *player) {
    player->idObjeto = ZAPATOS;
    player->ultimoObjetoClickeado = ZAPATOS;
    printf("\ZAPATOS CLICKEADO\n");
}
void interactuarConLaGuitarra(stJugador *player) {
    player->idObjeto = GUITARRA;
    player->ultimoObjetoClickeado = GUITARRA;
    printf("\GUITARRA CLICKEADO\n");
}
void interactuarConElArmario(stJugador *player) {
    player->idObjeto = ARMARIO;
    player->ultimoObjetoClickeado = ARMARIO;
    printf("\ARMARIO CLICKEADO\n");
}
void interactuarDentroDelTexto(stJugador *player) {
    player->idObjeto = player->ultimoObjetoClickeado;
}
void interactuarFueraDelTexto(stJugador *player) {
    player->textoActivo = 0;
}
void interactuarFueraDelSubEscenario(stJugador *player) {
    player->subEscenarioActivo = 0;
    printf("\n subEscenarioActivo = 0\n");
}
void interactuarConLaOreja(stJugador *player) {
    player->flags[OREJA].idProgreso = 1;
    printf("\n OREJA CLICKEADa\n");
}
void interactuarConLaAlmohada(stJugador *player) {
    player->idObjeto = ALMOHADA;
    player->ultimoObjetoClickeado = ALMOHADA;
    printf("\nALMOHADA CLICKEADO\n");
}
void interactuarConLaPuerta(stJugador *player) {
    player->idObjeto = PUERTA;
    player->ultimoObjetoClickeado = PUERTA;
    printf("\n PUERTA CLICKEADO \n");
}
void actualizarArbolHabitacion(stEscenario *escenario, stJugador *player) {
    int izquierda = 0;
    int derecha = 0;

    if( player->idObjeto == -1 ) printf("");

    else {
        stArbol *buscador = &(escenario[HABITACION].arbolDelEscenario);

        while( buscador != NULL && player->idObjeto != ((*buscador).fila->primero)->evento.idObjeto ) { ///BUSCANDO EL ID DEL OBJETO. el 3 debe coincidir con cantidad de objetos
            printf("\n izquierda %i", izquierda);
            buscador = buscador->izquierda;
            if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ) printf("\n player->idObjeto: %i", player->idObjeto);
            if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ) printf("\n ((*buscador).fila->primero)->evento.idObjeto: %i", ((*buscador).fila->primero)->evento.idObjeto);
            izquierda++;
        }
        if( buscador == NULL ) printf("\nNo se encontro el id objeto.\n");
        if( buscador != NULL ) buscador = buscador->derecha;

        while( buscador != NULL && player->flags[player->idObjeto].idProgreso != ((*buscador).fila->primero)->evento.idProgreso ) {
            printf("\n derecha %i", derecha);
            buscador = buscador->derecha;
            derecha++;
        }
        if( buscador == NULL ) printf("\nNo se encontro el id progreso.\n");
        if( buscador != NULL ) buscador = buscador->izquierda;

        if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ) { ///debug
            player->flags[player->idObjeto].idEvento = ((*buscador).fila->primero)->evento.idEvento; ///LINEA MAS IMPORTANTE DE TODO EL PROGRAMA
            printf("\n actualizar arbol idEvento: %i", player->flags[player->idObjeto].idEvento);
            printf("\n actualizar arbol idEvento: %i", player->flags[player->idObjeto].idEvento);
        }
        player->posicionSubArbol = buscador;
    }
}
void actualizarEventosHabitacion(stEscenario *escenario, stJugador *player) {

    if( player->idObjeto == -1 ) printf("");

    else {
        if( player->idObjeto == MUNIECA )   actualizarEventosHabitacionMunieca(player);
        if( player->idObjeto == ALFOMBRA )  actualizarEventosHabitacionAlfombra(player);
        if( player->idObjeto == CAJON )     actualizarEventosHabitacionCajon(player);
        if( player->idObjeto == ZAPATOS )   actualizarEventosHabitacionZapatos(player);
        if( player->idObjeto == GUITARRA )  actualizarEventosHabitacionGuitarra(player);
        if( player->idObjeto == ARMARIO )   actualizarEventosHabitacionArmario(player);
        if( player->idObjeto == ESPEJO )    actualizarEventosHabitacionEspejo(player);
        if( player->idObjeto == VENTANA )   actualizarEventosHabitacionVentana(player);
        if( player->idObjeto == CAMA )      actualizarEventosHabitacionCama(player);
        if( player->idObjeto == ALMOHADA )  actualizarEventosHabitacionAlmohada(player);
        if( player->idObjeto == PUERTA )    actualizarEventosHabitacionPuerta(player);
    }
}
void actualizarEventosHabitacionMunieca(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    Sound bufferSonido = ((*posicionArbol).fila->primero)->evento.sonido;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[player->idObjeto].idEvento == 2 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 2");
    }
    if( player->flags[player->idObjeto].idEvento == 3 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 3");
    }
    if( player->flags[player->idObjeto].idEvento == 4 ) {
        PlaySound( bufferSonido );
        inicializarEventosHabitacionMuniecaProgreso0(posicionArbol);
        player->flags[player->idObjeto].idProgreso = 1;
        player->textoActivo = 0;
        printf("\n se comprueba idevento 4");
    }
    if( player->flags[player->idObjeto].idEvento == 6 ) {
        inicializarEventosHabitacionMunieca6(posicionArbol);
        player->textoActivo = 1;
        printf("\n se comprueba idevento 6");
    }
    if( player->flags[player->idObjeto].idEvento == 8 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 8");
    }

    if( player->flags[player->idObjeto].idEvento == 9 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 9");
    }
    if( player->flags[player->idObjeto].idEvento == 10 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 10");
    }
    if( player->flags[player->idObjeto].idEvento == 11 ) {
        PlaySound( bufferSonido );
        player->flags[player->idObjeto].idProgreso = 3;
        inicializarEventosHabitacionMuniecaProgreso2(posicionArbol);
        player->textoActivo = 0;
        printf("\n se comprueba idevento 11");
    }
    if( player->flags[player->idObjeto].idEvento == 13 ) {
        PlaySound(bufferSonido);
        player->textoActivo = 0;
        printf("\n se comprueba idevento 13");
    }
    if( player->flags[player->idObjeto].idEvento == 14 ) {
        PlaySound(bufferSonido);
        player->textoActivo = 0;
        printf("\n se comprueba idevento 14");
    }
    if( player->flags[player->idObjeto].idEvento == 15 ) {
        PlaySound(bufferSonido);
        inicializarEventosHabitacionMuniecaProgreso3(posicionArbol);
        player->flags[ARMARIO].idProgreso = 0;
        player->textoActivo = 0;
        printf("\n se comprueba idevento 15");
    }
}
void actualizarEventosHabitacionAlfombra(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[player->idObjeto].idEvento == 22 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 22");
    }
    if( player->flags[player->idObjeto].idEvento == 23 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 23");
    }
    if( player->flags[player->idObjeto].idEvento == 24 ) {
        inicializarEventosHabitacionAlfombraProgreso0(posicionArbol);
        player->flags[player->idObjeto].idProgreso = 1;
        player->flags[MUNIECA].idProgreso = 2;
        player->textoActivo = 1;
        printf("\n se comprueba idevento 24");
    }
    if( player->flags[player->idObjeto].idEvento == 26 ) {
        inicializarEventosHabitacionAlfombra26(posicionArbol);
        player->textoActivo = 1;
        printf("\n se comprueba idevento 26");
    }
}
void actualizarEventosHabitacionCajon(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[player->idObjeto].idEvento == 32 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 32");
    }
    if( player->flags[player->idObjeto].idEvento == 33 ) {
        inicializarEventosHabitacionCajon33(posicionArbol);
        player->flags[ZAPATOS].idProgreso = 1;
        player->textoActivo = 1;
        printf("\n se comprueba idevento 33");
    }
    if( player->flags[player->idObjeto].idEvento == 35 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 35");
    }
    if( player->flags[player->idObjeto].idEvento == 36 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 36");
    }
    if( player->flags[player->idObjeto].idEvento == 37 ) {
        inicializarEventosHabitacionCajonProgreso1(posicionArbol);
        player->flags[player->idObjeto].idProgreso = 2;
        player->textoActivo = 1;
        printf("\n se comprueba idevento 37");
    }
    if( player->flags[player->idObjeto].idEvento == 39 ) {
        inicializarEventosHabitacionCajon39(posicionArbol);
        player->flags[GUITARRA].idProgreso = 1;
        player->textoActivo = 1;
        printf("\n se comprueba idevento 39 \n");
    }
}
void actualizarEventosHabitacionZapatos(stJugador *player)  {
    stArbol *posicionArbol = player->posicionSubArbol;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[CAJON].idEvento == 33 ) {
        if( player->flags[player->idObjeto].idEvento == 43 ) {
            player->textoActivo = 1;
            printf("\n se comprueba idevento 43 \n");
        }
        if( player->flags[player->idObjeto].idEvento == 44 ) {
            inicializarEventosHabitacionZapatosProgreso1(posicionArbol);
            player->flags[player->idObjeto].idProgreso = 2;
            player->textoActivo = 1;
            printf("\n se comprueba idevento 44 \n");
        }
        if( player->flags[player->idObjeto].idEvento == 46 ) {
            inicializarEventosHabitacionZapatos46(posicionArbol);
            player->flags[CAJON].idProgreso = 1;
            player->textoActivo = 1;
            printf("\n se comprueba idevento 46 \n");
        }
    }
}
void actualizarEventosHabitacionGuitarra(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    Sound bufferSonido = ((*posicionArbol).fila->primero)->evento.sonido;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[CAJON].idEvento == 39 ) {
        if( player->flags[player->idObjeto].idEvento == 52 ) {
            player->textoActivo = 1;
            printf("\n se comprueba idevento 52");
        }
        if( player->flags[player->idObjeto].idEvento == 53 ) {
            PlaySound( bufferSonido );
            player->textoActivo = 0;
            printf("\n se comprueba idevento 53");
        }
        if( player->flags[player->idObjeto].idEvento == 54 ) {
            inicializarEventosHabitacionGuitarraProgreso1(posicionArbol);
            player->flags[player->idObjeto].idProgreso = 2;
            player->textoActivo = 1;
            printf("\n se comprueba idevento 54");
        }
        if( player->flags[player->idObjeto].idEvento == 56 ) {
            inicializarEventosHabitacionGuitarra56(posicionArbol);
    ///        player->flags[CANDADOPUERTA].idProgreso = ; HABILITA UN PROGRESO DEL CANDADO
            player->textoActivo = 1;
            printf("\n se comprueba idevento 56");
        }
    }
}
void actualizarEventosHabitacionArmario(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    Sound bufferSonido = ((*posicionArbol).fila->primero)->evento.sonido;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[MUNIECA].idEvento == 13 || player->flags[MUNIECA].idEvento == 14 || player->flags[MUNIECA].idEvento == 15 ) {

        if( player->flags[player->idObjeto].idEvento == 72 ) {
            player->textoActivo = 1;
            printf("\n se comprueba idevento 72");
        }
        if( player->flags[player->idObjeto].idEvento == 73 ) {
            inicializarEventosHabitacionArmarioProgreso0(posicionArbol);
            PlaySound( bufferSonido );
            player->flags[player->idObjeto].idProgreso = 1;
            player->textoActivo = 0;
            printf("\n se comprueba idevento 73");
        }
        if( player->flags[player->idObjeto].idEvento == 75 ) {
            player->textoActivo = 1;
            printf("\n se comprueba idevento 75");
        }
        if( player->flags[player->idObjeto].idEvento == 76 ) {
            inicializarEventosHabitacionArmarioProgreso1(posicionArbol);
    ///        player->flags[CANDADOPUERTA].idProgreso = ; HABILITA UN PROGRESO DEL CANDADO
            player->textoActivo = 1;
            printf("\n se comprueba idevento 76");
        }
    }
}
void actualizarEventosHabitacionEspejo(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->idObjeto].idEvento == 62 ) {
            player->subEscenarioActivo = 2;
            printf("\n se comprueba idevento 62");
    }
}
void actualizarEventosHabitacionVentana(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->idObjeto].idEvento == 82 ) {
            player->subEscenarioActivo = 1;
            printf("\n se comprueba idevento 82");
    }
}
void actualizarEventosHabitacionCama(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    Sound bufferSonido = ((*posicionArbol).fila->primero)->evento.sonido;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[player->idObjeto].idEvento == 52 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 52 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 53 ) {
        player->textoActivo = 1;
        if( player->flags[OREJA].idProgreso == 1 && player->flags[ALMOHADA].idProgreso == 1 )
            player->flags[player->idObjeto].idProgreso = 1;
        inicializarEventosHabitacionCamaProgreso0(posicionArbol);
        printf("\n se comprueba idevento 53");
    }
    if( player->flags[player->idObjeto].idEvento == 55 ) {
        player->textoActivo = 0;
        //PlaySound( bufferSonido ); ///AUDIO
        player->flags[player->idObjeto].idProgreso = 2;
        printf("\n se comprueba idevento 55 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 57 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionCama57(posicionArbol);
        printf("\n se comprueba idevento 57 \n");
    }
}
void actualizarEventosHabitacionAlmohada(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    Sound bufferSonido = ((*posicionArbol).fila->primero)->evento.sonido;

    quitarPrimerNodoFila(posicionArbol->fila);

    if( player->flags[player->idObjeto].idEvento == 92 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 92 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 93 ) {
        player->textoActivo = 0;
        //PlaySound( bufferSonido ); ///AUDIO
        printf("\n se comprueba idevento 93 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 94 ) {
        player->textoActivo = 1;
        printf("\n se comprueba idevento 94 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 95 ) {
        player->textoActivo = 0;
        //PlaySound( bufferSonido ); ///AUDIO
        printf("\n se comprueba idevento 95 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 96 ) {
        player->textoActivo = 0;
        //PlaySound( bufferSonido ); ///AUDIO
        printf("\n se comprueba idevento 96 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 97 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionAlmohadaProgreso0(posicionArbol);
        player->flags[player->idObjeto].idProgreso = 1;
        printf("\n se comprueba idevento 97 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 99 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionAlmohada99(posicionArbol);
        printf("\n se comprueba idevento 99 \n");
    }
}
void actualizarEventosHabitacionPuerta(stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    quitarPrimerNodoFila(posicionArbol->fila);

    int contadorDeLlaves = 0;

    if( player->flags[ARMARIO].idProgreso == 1 ) contadorDeLlaves++;
    if( player->flags[CAMA].idProgreso == 2 ) contadorDeLlaves++;
    if( player->flags[GUITARRA].idProgreso == 2 ) contadorDeLlaves++;

    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ) printf("\n contadorDeLlaves: %i \n", contadorDeLlaves); ///DEBUG

    if( player->flags[player->idObjeto].idEvento == 102 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionPuerta102(posicionArbol);
        ///CONDICIONES PARA ABRIR LA PUERTA
        if( contadorDeLlaves == 1 )
            player->flags[player->ultimoObjetoClickeado].idProgreso = 1; ///DEBUG
        printf("\n se comprueba idevento 102 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 104 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionPuerta104(posicionArbol);
        ///CONDICIONES PARA ABRIR LA PUERTA
        if( contadorDeLlaves == 2 )
            player->flags[player->ultimoObjetoClickeado].idProgreso = 2; ///DEBUG
        printf("\n se comprueba idevento 104 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 106 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionPuerta106(posicionArbol);
        ///CONDICIONES PARA ABRIR LA PUERTA
        if( contadorDeLlaves == 2 )
            player->flags[player->ultimoObjetoClickeado].idProgreso = 3; ///DEBUG
        printf("\n se comprueba idevento 106 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 108 ) {
        player->textoActivo = 1;
        inicializarEventosHabitacionPuerta108(posicionArbol);
        ///CONDICIONES PARA ABRIR LA PUERTA
        player->flags[player->ultimoObjetoClickeado].idProgreso = 4; ///DEBUG
        printf("\n se comprueba idevento 108 \n");
    }
    if( player->flags[player->idObjeto].idEvento == 110 ) {
        player->escenario = GANASTE;
        inicializarEventosHabitacionPuerta110(posicionArbol);
        printf("\n se comprueba idevento 110 \n");
    }
}
void actualizarScore(stJugador *player) {
    player->score--;
    if( player->score == 0 ) {
        gameOver(player);
}
void gameOver(stJugador *player) {
    player->escenario = GAMEOVER;
    PlaySound(player->gameOverSound);

}
//void actualizarInputCocina(stEscenario *escenario, stJugador *player){
//    if( seClickeaElCuadro(escenario) )   interactuarConElCuadro(escenario, player);
//    if( seClickeaElTacho(escenario) )    interactuarConElEspejo(escenario, player);
//    if( seClickeaLaHeladera(escenario) ) interactuarConLaHeladera(escenario, player);
//    if( seClickeaElCuchillo(escenario) ) interactuarConElCuchillo(escenario, player);
//    if( seClickeaLaPared(escenario) )    interactuarConLaPared(escenario, player);
//    if( seClickeaLaMesada(escenario) )   interactuarConLaMesada(escenario, player);
//    if( seClickeaLaPuerta(escenario) )   interactuarConLaPuerta(escenario, player);
//}

//int seClickeaElCuadro(stEscenario *escenario) {
//    int esClickeado = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[CUADRO].hitbox) )
//        esClickeado = 1;
//    return esClickeado;
//}
//int seClickeaElTacho(stEscenario *escenario) {
//    int esClickeado = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[TACHO].hitbox) )
//        esClickeado = 1;
//    return esClickeado;
//}
//int seClickeaLaHeladera(stEscenario *escenario) {
//    int esClickeada = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[HELADERA].hitbox) )
//        esClickeada = 1;
//    return esClickeada;
//}
//int seClickeaElCuchillo(stEscenario *escenario) {
//    int esClickeado = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[CUCHILLO].hitbox) )
//        esClickeado = 1;
//    return esClickeado;
//}
//int seClickeaLaPared(stEscenario *escenario) {
//    int esClickeada = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[PARED].hitbox) )
//        esClickeada = 1;
//    return esClickeada;
//}
//int seClickeaLaMesada(stEscenario *escenario) {
//    int esClickeada = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[MESADA].hitbox) )
//        esClickeada = 1;
//    return esClickeada;
//}
//int seClickeaLaPuerta(stEscenario *escenario) {
//    int esClickeada = 0;
//    Vector2 posicionDelMouse = GetMousePosition();
//    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[PUERTA].hitbox) )
//        esClickeada = 1;
//    return esClickeada;
//}
//void interactuarConElCuadro(stEscenario *escenario, stJugador *player) {
//    player->idObjeto = CUADRO;
//    printf("\nCUADRO CLICKEADO\n");
//}
//void interactuarConElTacho(stEscenario *escenario, stJugador *player) {
//    player->idObjeto = TACHO;
//    printf("\nTACHO CLICKEADO\n");
//}
//void interactuarConLaHeladera(stEscenario *escenario, stJugador *player) {
//    player->idObjeto = HELADERA;
//    printf("\nHELADERA CLICKEADO\n");
//}
//void interactuarConElCuchillo(stEscenario *escenario, stJugador *player) {
//    player->idObjeto = CUCHILLO;
//    printf("\nCUCHILLO CLICKEADO\n");
//}
//void interactuarConLaPared(stEscenario *escenario, stJugador *player) {
//    player->idObjeto = PARED;
//    printf("\nPARED CLICKEADO\n");
//}
//void interactuarConLaMesada(stEscenario *escenario, stJugador *player) {
//    player->idObjeto = MESADA;
//    printf("\nMESADA CLICKEADO\n");
//}
void actualizarInputMenu(stEscenario *escenario, stJugador *player) {
    if( seClickeaElBotonPlay(escenario) )    interactuarConElBotonPlay(escenario, player);
    if( seClickeaElBotonLoad(escenario) )    interactuarConElBotonLoad(escenario, player);
    if( seClickeaElBotonScore(escenario) )   interactuarConElBotonScore(escenario, player);
    if( seClickeaElBotonSave(escenario) )    interactuarConElBotonSave(escenario, player);
    if( seClickeaElBotonCredits(escenario) ) interactuarConElBotonCredits(escenario, player);
}
int seClickeaElBotonPlay(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[MENU].objetos[PLAY].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaElBotonLoad(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[MENU].objetos[BOTONLOAD].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaElBotonScore(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[MENU].objetos[BOTONSCORE].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaElBotonSave(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[MENU].objetos[BOTONSAVE].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaElBotonCredits(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[MENU].objetos[BOTONCREDITS].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
void interactuarConElBotonPlay(stEscenario *escenario, stJugador *player) {
    player->escenario = HABITACION;
    ///printf("\nBOTON PLAY CLICKEADO\n"); ///DEBUG
}
void interactuarConElBotonLoad(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BOTONLOAD;
    printf("\nBOTON LOAD CLICKEADO\n");
}
void interactuarConElBotonScore(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BOTONSCORE;
    printf("\nBOTON SCORE CLICKEADO\n");
    player->escenario = SCORE;
}
void interactuarConElBotonSave(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BOTONSAVE;
    printf("\nBOTON SAVE CLICKEADO\n");
}
void interactuarConElBotonCredits(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BOTONCREDITS;
    player->escenario = CREDITOS;
    printf("\nBOTON CREDITS CLICKEADO\n");
}
void actualizarInputCreditos(stEscenario *escenario, stJugador *player) {
      if( seClickeaElBotonBack(escenario) )  interactuarConElBotonBack(escenario, player);
}
void actualizarPosicionCreditos(stJugador *player) {
    player->scrollText++;
    if( player->scrollText == 1100 ) player->scrollText = -1100;
}
int seClickeaElBotonBack(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[CREDITOS].objetos[BACK].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
void interactuarConElBotonBack(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BACK;
    player->escenario = MENU;
    printf("\nBOTON BACK CLICKEADO\n");
}
void actualizarInputGanaste(stEscenario *escenario, stJugador *player) {
      if( seClickeaLaPantalla(escenario) )  interactuarConLaPantalla(escenario, player);
}
int seClickeaLaPantalla(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[GANASTE].objetos[BOTONGANASTE].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
void interactuarConLaPantalla(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BOTONGANASTE;
    player->escenario = MENU;
    printf("\nBOTON GANASTE CLICKEADO\n");
}
void actualizarInputBackScore(stEscenario *escenario, stJugador *player) {
    if( seClickeaBotonBackScore(escenario) )  interactuarConElBotonBackScore(escenario, player);
}
int seClickeaBotonBackScore(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[SCORE].objetos[BOTONSCOREBACK].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
void actualizarInputSave(stJugador *player) {
    if( seClickeaElBotonGuardar(player) ) guardarArchivoSave(player);
    //if( seClickeaElBotonCargar1(Player) ) cargarArchivoSave(player, 1);
    //if( seClickeaElBotonCargar2(Player) ) cargarArchivoSave(player, 2);
}
void interactuarConElBotonBackScore(stEscenario *escenario, stJugador *player) {
    player->idObjeto = BOTONSCOREBACK;
    player->escenario = MENU;
    printf("\nBOTON BACK SCORE CLICKEADO\n");
}
void inputSaveScreen(stJugador *player) {
    player->switchTimer++;
    if( player->switchTimer%45 == 0 )
        player->switchState = ( player->switchState == 0 ) ? 1 : 0;
    cargarNombreDinamico(player);
    if( IsKeyPressed(KEY_ENTER) && player->saveGuardado == 0 ) guardarArchivoScore(player);
}
void cargarNombreDinamico(stJugador *player) {
    int letra = GetCharPressed();
    if( player->saveGuardado == 0 ) {
        while( letra > 0 ) {
            cargarLetraDinamica(player, letra);
            letra = GetCharPressed();
        }
        if( IsKeyPressed(KEY_BACKSPACE) ) borrarLetraDinamica(player);
    }
}
void cargarLetraDinamica(stJugador *player, int letra) {
    if(  letra <= 125 && letra >= 32 && player->posicionLetra < DIM && letra != 53 ) {
        player->nombre[player->posicionLetra] = (char)letra;
        player->nombre[player->posicionLetra+1] = '\0';
        player->posicionLetra++;
    }
}
void borrarLetraDinamica(stJugador *player) {
    player->posicionLetra--;
    if (player->posicionLetra < 0) player->posicionLetra = 0;
    player->nombre[player->posicionLetra] = '\0';
}
void guardarArchivoSave(stJugador *player) {
    player->saveGuardado = 1;
    stJugador bufferSave;
    FILE *archivoSave = fopen(nombreArchivoSave, "ab");
    if( archivoSave ==  NULL ) printf("\nError. El archivo de Save no existe.");
    if( archivoSave !=  NULL ) {
        copiarSave(&bufferSave, player);
        fwrite(&bufferSave, sizeof(stJugador), 1, archivoSave);
        fclose(archivoSave);
    }
}
int calcularValidosArchivoScores(stJugador *player) {
    int validos = 0;
    FILE *archivoSave = fopen(nombreArchivoSave, "ab");
    if( archivoSave ==  NULL ) printf("\nError. El archivo de Save no existe.");
    if( archivoSave !=  NULL ) {
        fseek(archivoSave, 0, SEEK_END);
        validos = ftell(archivoSave) / sizeof(stJugador);
        fclose(archivoSave);
    }
    return validos;
}
///DIBUJAR
void dibujar(stEscenario *escenario, stJugador *player, Debug *herramientaDebug) {
    BeginDrawing();
    ClearBackground(BLACK);

    if( player->escenario == MENU )       dibujarMenu(escenario, player);
    if( player->escenario == HABITACION ) dibujarHabitacion(escenario, player);
    if( player->escenario == CREDITOS )   dibujarCreditos(escenario, player);
    if( player->escenario == GANASTE )    dibujarGanaste(escenario); ///TOQUE ACA
    if( player->escenario == SAVE )       dibujarScores(player); ///TOQUE ACA
    if( player->escenario == SCORE )      dibujarScore(escenario, player); ///TOQUE ACA
    if( player->escenario == GAMEOVER )   dibujarGameOver(escenario, player); ///TOQUE ACA

    if( herramientaDebug->mostrarHerramienta == 1 ) dibujarVentanaDebug(herramientaDebug);
    actualizarTestDebug(escenario, player, herramientaDebug);

    EndDrawing(); ///Cierra las funciones para dibujar
}
void dibujarMenu(stEscenario *escenario, stJugador *player) {
    DrawTexture(escenario[MENU].fondoEstatico, 500, 0, WHITE);
    DrawTexture(escenario[MENU].botones[PLAY], 880, 750, WHITE);//PLAY
    DrawTexture(escenario[MENU].botones[BOTONLOAD], 550, 700, WHITE);//LOAD
    DrawTexture(escenario[MENU].botones[BOTONSCORE], 550, 800, WHITE);//SCORE
    DrawTexture(escenario[MENU].botones[BOTONSAVE], 1260, 700, WHITE);//SAVE
    DrawTexture(escenario[MENU].botones[BOTONCREDITS], 1260, 800, WHITE);//CREDITS
    dibujarBotonesMenu(escenario);
}
void dibujarBotonesMenu(stEscenario *escenario) {
    dibujarBotonPlay(escenario);
    dibujarBotonLoad(escenario);
    dibujarBotonSave(escenario);
    dibujarBotonScore(escenario);
    dibujarBotonCredits(escenario);
}
void dibujarBotonPlay(stEscenario *escenario) {
    DrawText("PLAY",935,829,60, BLACK);
}
void dibujarBotonLoad(stEscenario *escenario) {
    DrawText("LOAD",605,712,40, BLACK);
}
void dibujarBotonScore(stEscenario *escenario) {
    DrawText("SCORE",590,810,40, BLACK);
}
void dibujarBotonSave(stEscenario *escenario) {
    DrawText("SAVE",1310,712,40, BLACK);
}
void dibujarBotonCredits(stEscenario *escenario) {
    DrawText("CREDITS",1280,810,39, BLACK);
}
void dibujarHabitacion(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    DrawTexture(escenario[HABITACION].fondoEstatico, 500, 0, WHITE);

    if( player->textoActivo == 1 ) {
        if( player->ultimoObjetoClickeado == MUNIECA )  dibujarHabitacionMunieca(escenario, player);
        if( player->ultimoObjetoClickeado == ALFOMBRA ) dibujarHabitacionAlfombra(escenario, player);
        if( player->ultimoObjetoClickeado == CAJON )    dibujarHabitacionCajon(escenario, player);
        if( player->ultimoObjetoClickeado == ZAPATOS )  dibujarHabitacionZapatos(escenario, player);
        if( player->ultimoObjetoClickeado == GUITARRA ) dibujarHabitacionGuitarra(escenario, player);
        if( player->ultimoObjetoClickeado == ARMARIO )  dibujarHabitacionArmario(escenario, player);
        if( player->ultimoObjetoClickeado == CAMA )     dibujarHabitacionCama(escenario, player);
        if( player->ultimoObjetoClickeado == ALMOHADA ) dibujarHabitacionAlmohada(escenario, player);
        if( player->ultimoObjetoClickeado == PUERTA )   dibujarHabitacionPuerta(escenario, player);
    }
    if( player->subEscenarioActivo == 1 )
        DrawTexture(escenario[HABITACION].subEscenarios[1], 500, 0, WHITE); ///Subescenario ventana, <- estos comentarios son un problema, hay q indexar y no comentar ej: subEscenarios[SUBVENTANA];
    if( player->subEscenarioActivo == 2 )
        DrawTexture(escenario[HABITACION].subEscenarios[0], 500, 0, WHITE); ///Subescenario espejo, <- estos comentarios son un problema, hay q indexar y no comentar ej: subEscenarios[SUBESPEJO];
//    if( player->objetoActivo != 0 ) { ///funcion para mostrar los objetos obtenidos hasta ahora
//
//    }
}
void dibujarHabitacionMunieca(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 2 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La munieca yace inerte, su rostro de porcelana estatico.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("Algo en su interior parece haber perdido su brillo.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("Hay algo que puedas hacer para despertarla?", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 3 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La munieca revela un hueco en la espalda,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("como si esperara un objeto para cobrar vida.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 6 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("El toque despierta su misterio, pero sin ", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("energia su encanto queda en la sombra.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("Aguarda, lista para cobrar vida.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 8 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Las pilas encajan perfectamente en la munieca,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("pero la quietud persiste.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 9 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Al observar detenidamente, descubres que la", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("munieca esconde un boton oculto.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("Prueba encenderla y revelar los secretos.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 10 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La munieca comienza a emitir sonidos desde su boca,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("suenan tenue. Por que no te acercas?", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
}
void dibujarHabitacionAlfombra(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 22 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La alfombra revela un misterio bajo", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("sus telas rotas y manchadas.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 23 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Despliegas la alfombra y...", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("descubres un secreto oculto.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 24 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Unas pilas asoman timidamente,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("como si esperaran ser encontradas.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 26 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Has encontrado algo mas que simples pilas,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("o hay algo mas oscuro aguardando su despertar?", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
}
void dibujarHabitacionCajon(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 32 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Parece que el cajon esta bloqueado con una combinacion.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("Una punta de papel sobresale, sugiriendo que", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("hay algo mas en su interior...", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 33 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Tirando de la punta de la hoja.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 50, 30, RED);
        DrawText("Logras sacarla sin haber abierto el cajon.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 100, 30, RED);
        DrawText("En la nota se lee: ", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 150, 30, RED);
        DrawText(" 'Presta atencion a los detalles bajo tus pies'", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 200, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 35 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Ingresando la clave descubierta en el zapato,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("logras abrir el candado..", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 36 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La combinacion desbloqueo el cajon,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("revelando una nota antigua.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("Parece ser la clave de algo mas...", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 37 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Se revela una partitura que parece.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("emanar una extrania energia.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 39 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Te atreverias a liberar las fuerzas ocultas que", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("aguardan en las sombras de esta composicion misteriosa?", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
}
void dibujarHabitacionZapatos(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 43 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Examinas unos zapatos y descubres", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("una mancha fresca color rojo.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 44 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Revisando ambos zapatos te das cuenta de", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 50, 30, RED);
        DrawText("que uno de ellos tiene numeros escritos.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 100, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 46 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Los numeros resultan ser: 666", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 100, 50, RED);
    }
}
void dibujarHabitacionGuitarra(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 52 ) {

        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Al tocar la guitarra.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("Emite una melodia por si sola…", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 54 ) {

        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Un compartimento secreto se revela.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 50, 30, RED);
        DrawText(" y en su interior hay una llave inusual.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 56 ) {

        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La llave parece abrir una cerradura de la puerta", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
}
void dibujarHabitacionArmario(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 72 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Abriendo el espeluznante armario,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText(" se puede notar una presencia oscura.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 75 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Vez una llave en el fondo del armario y la agarras.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 76 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La llave parece abrir un candado de la puerta.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
}
void dibujarHabitacionCama(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 52 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Esta cama guarda los inquietantes recuerdos", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("de quienes descansaron aqui, cada suenio", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("alberga sus propios secretos de terror.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 53 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Los susurros espeluznantes flotan en la almohada,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("como ecos de almas atrapadas.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 57 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Has encontrado una llave vieja y oxidada", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("debajo de las sabanas de la cama.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("Que abrira?", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
}
void dibujarHabitacionAlmohada(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 92 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Se puede escuchar como una voz", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("bajita sale de la almohada.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 94 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Has encontrado un dedo arrancado", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("de un humano en descomposicion", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("debajo de la almohada.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 97 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("A simple vista se puede notar como", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("la luna llena resalta en la habitacion.", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 99 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Ya encontraste el trozo humano", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("debajo de la almohada,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("sigue revisando la habitacion…", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
}
void dibujarHabitacionPuerta(stEscenario *escenario, stJugador *player) {
    stArbol *posicionArbol = player->posicionSubArbol;

    if( player->flags[player->ultimoObjetoClickeado].idEvento == 102 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("La puerta parece estar cerrada con tres", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("cerraduras, en algun lugar de la habitacion", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("deben estar las llaves que la abren…", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 104 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Abriste una de las cerraduras con la llave,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("pero aun quedan dos mas y pareciera ", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("que cada vez hay menos tiempo…", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 106 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Lograste abrir otra de las cerraduras,", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("solo queda una mas, puede que logres", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
        DrawText("salir antes que lleguen…", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 170, 30, RED);
    }
    if( player->flags[player->ultimoObjetoClickeado].idEvento == 108 ) {
        DrawTexture(escenario[HABITACION].subEscenarios[TEXTO], escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, WHITE);
        DrawText("Has logrado escapar", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 70, 30, RED);
        DrawText("de la habitacion con vida!", escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x + 90, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y + 120, 30, RED);
    }
}
void dibujarCreditos(stEscenario *escenario, stJugador *player) {
    DrawTexture(escenario[CREDITOS].fondoEstatico, 500, 0, WHITE);
    DrawTexture(escenario[CREDITOS].botones[BACK], 875, 950, WHITE);//LOAD
    dibujarBotonBackCreditos(escenario);
    dibujarScrollCreditos(escenario, player);
}
void dibujarGanaste(stEscenario *escenario) {
    DrawTexture(escenario[GANASTE].fondoEstatico, 500, 0, WHITE); ///TOQUE
}
void dibujarScore(stEscenario *escenario) {         ///TOQUE
    DrawTexture(escenario[SCORE].fondoEstatico, 500, 0, WHITE);
    DrawTexture(escenario[SCORE].botones[BOTONSCOREBACK], 929, 897, WHITE);
    dibujarBotonBackScore(escenario);
}
void dibujarGameOver(stEscenario *escenario) {         ///TOQUE
    DrawTexture(escenario[GAMEOVER].fondoEstatico, 500, 0, WHITE);
    DrawTexture(escenario[GAMEOVER].botones[BOTONSCOREBACK], 929, 897, WHITE);
    dibujarBotonBackScore(escenario);
}
void dibujarBotonBackScore(stEscenario *escenario) { ///TOQUE
    DrawText("BACK",983,908,39, BLACK);
}
void dibujarBotonBackCreditos(stEscenario *escenario) {
    DrawText("BACK",930,960,39, BLACK);
}
void dibujarScrollCreditos(stEscenario *escenario, stJugador *player) {
    DrawText("Desarrolladores", 700, 100 - player->scrollText, 60, YELLOW);
    DrawText("Ortiz Rocio", 700, 200 - player->scrollText, 60, WHITE);
    DrawText("Ortiz Brisa", 700, 300 - player->scrollText, 60, WHITE);
    DrawText("Olmos Fernando", 700,400 - player->scrollText, 60, WHITE);
    DrawText("Docente y Ayudantes",700, 500 - player->scrollText, 60, YELLOW);
    DrawText("Adrian Aroca", 700, 600 - player->scrollText, 60, WHITE);
    DrawText("Hilario Monti", 700, 700 - player->scrollText, 60, WHITE);
    DrawText("Nicanor Dondero", 700, 800 - player->scrollText, 60 , WHITE);
}
void deinicializaciones() {
    CloseAudioDevice();
    CloseWindow();
}
///Funciones que deben de estar en librería
void crearNodoFila(stNodo **nuevoNodo, stEvento evento) {
    (*nuevoNodo)->evento = evento;
    (*nuevoNodo)->siguiente = NULL;
    (*nuevoNodo)->anterior = NULL;
}
void crearNodoArbol(stArbol **arbol){
    stArbol *nuevoArbol = (stArbol *)malloc(sizeof(stArbol));
    nuevoArbol->derecha = NULL;
    nuevoArbol->izquierda = NULL;
    *arbol = nuevoArbol;
}
void mostrarArbol(stArbol *arbol) {
    if(arbol != NULL) {
        printf("\n----------\n");
        printf("\nPUNTERO ARBOL %p\n", arbol);
        if( arbol != NULL )
            printf("\n ((*arbol).fila->primero)->evento.idEvento: %i\n", ((*arbol).fila->primero)->evento.idEvento);
        printf("\nPUNTERO ARBOL IZQUIERDA %p\n", arbol->izquierda);
        printf("\nPUNTERO ARBOL DERECHA %p\n", arbol->derecha);
        mostrarArbol(arbol->izquierda);
        mostrarArbol(arbol->derecha);
    }
}
void agregarNodoEnFila(stFila *fila, stEvento evento) {
    stNodo *nuevoNodo = (stNodo*) malloc(sizeof(stNodo));
    crearNodoFila(&nuevoNodo, evento);

    if(fila->primero == NULL) {
        fila->primero = nuevoNodo;
        fila->ultimo = nuevoNodo;
    }else{
        fila->ultimo->siguiente = nuevoNodo;
        nuevoNodo->anterior = fila->ultimo;
        fila->ultimo = nuevoNodo;
    }
}
void mostrarNodo(stNodo *nodo) {
    printf("\n--------------------------------------------------");
    printf("\n idObjeto: %i", nodo->evento.idObjeto);
    printf("\n idEvento: %i", nodo->evento.idEvento);
    printf("\n activo: %i", nodo->evento.activo);
    printf("\n texto: %s", nodo->evento.texto);
    printf("\n anterior: %p", nodo->anterior);
    printf("\n actual: %p", nodo);
    printf("\n siguiente: %p", nodo->siguiente);
    printf("\n--------------------------------------------------");
}
void mostrarFila(stFila fila) {
    stNodo *indiceFila = fila.primero;
    if( indiceFila == NULL )
        printf("\nLa fila esta vacia.");
    if( indiceFila != NULL ) {
        while( indiceFila != NULL ) {
            mostrarNodo(indiceFila);
            indiceFila = indiceFila->siguiente;
        }
    }
}
void quitarPrimerNodoFila(stFila *fila) {
    if (fila == NULL || fila->primero == NULL)  return;

    stNodo *nodoBorrar = fila->primero;

    fila->primero = nodoBorrar->siguiente;

    if (fila->primero != NULL) fila->primero->anterior = NULL;

    else fila->ultimo = NULL;

    free(nodoBorrar);
    nodoBorrar = NULL;
}
void actualizarTestDebug(stEscenario *escenario, stJugador *player, Debug *herramientaDebug) {
    ///ZONA DE TESTEO DE ACTUALIZACION--------------------------------------
    ///las funciones de dibujado estan en actualizar porque en dibujar no funcionan, teniendo el mismo formato y funciones
    if( player->escenario == HABITACION ) {
        if( player->textoActivo == 0 && player->subEscenarioActivo == 0 ) { ///HITBOXES
            DrawRectangleLines(escenario[HABITACION].objetos[CAMA].hitbox.x, escenario[HABITACION].objetos[CAMA].hitbox.y, escenario[HABITACION].objetos[CAMA].hitbox.width, escenario[HABITACION].objetos[CAMA].hitbox.height, PURPLE);
            DrawRectangleLines(escenario[HABITACION].objetos[VENTANA].hitbox.x, escenario[HABITACION].objetos[VENTANA].hitbox.y, escenario[HABITACION].objetos[VENTANA].hitbox.width, escenario[HABITACION].objetos[VENTANA].hitbox.height, YELLOW);
            DrawRectangleLines(escenario[HABITACION].objetos[CAJON].hitbox.x, escenario[HABITACION].objetos[CAJON].hitbox.y, escenario[HABITACION].objetos[CAJON].hitbox.width, escenario[HABITACION].objetos[CAJON].hitbox.height, YELLOW);
            DrawRectangleLines(escenario[HABITACION].objetos[ALMOHADA].hitbox.x, escenario[HABITACION].objetos[ALMOHADA].hitbox.y, escenario[HABITACION].objetos[ALMOHADA].hitbox.width, escenario[HABITACION].objetos[ALMOHADA].hitbox.height, GREEN);
            DrawRectangleLines(escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.x, escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.y, escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.width, escenario[HABITACION].objetos[CANDADOPUERTA].hitbox.height, RED);
            DrawRectangleLines(escenario[HABITACION].objetos[MUNIECA].hitbox.x, escenario[HABITACION].objetos[MUNIECA].hitbox.y, escenario[HABITACION].objetos[MUNIECA].hitbox.width, escenario[HABITACION].objetos[MUNIECA].hitbox.height, BLUE);
            DrawRectangleLines(escenario[HABITACION].objetos[ALFOMBRA].hitbox.x, escenario[HABITACION].objetos[ALFOMBRA].hitbox.y, escenario[HABITACION].objetos[ALFOMBRA].hitbox.width, escenario[HABITACION].objetos[ALFOMBRA].hitbox.height, RED);
            DrawRectangleLines(escenario[HABITACION].objetos[ESPEJO].hitbox.x, escenario[HABITACION].objetos[ESPEJO].hitbox.y, escenario[HABITACION].objetos[ESPEJO].hitbox.width, escenario[HABITACION].objetos[ESPEJO].hitbox.height, YELLOW);
            DrawRectangleLines(escenario[HABITACION].objetos[ZAPATOS].hitbox.x, escenario[HABITACION].objetos[ZAPATOS].hitbox.y, escenario[HABITACION].objetos[ZAPATOS].hitbox.width, escenario[HABITACION].objetos[ZAPATOS].hitbox.height, ORANGE);
            DrawRectangleLines(escenario[HABITACION].objetos[GUITARRA].hitbox.x, escenario[HABITACION].objetos[GUITARRA].hitbox.y, escenario[HABITACION].objetos[GUITARRA].hitbox.width, escenario[HABITACION].objetos[GUITARRA].hitbox.height, GREEN);
            DrawRectangleLines(escenario[HABITACION].objetos[ARMARIO].hitbox.x, escenario[HABITACION].objetos[ARMARIO].hitbox.y, escenario[HABITACION].objetos[ARMARIO].hitbox.width, escenario[HABITACION].objetos[ARMARIO].hitbox.height, ORANGE);
        }
        if( player->textoActivo == 1 ) {
            DrawRectangleLines(escenario[HABITACION].objetos[DENTROTEXTO].hitbox.x, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.y, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.width, escenario[HABITACION].objetos[DENTROTEXTO].hitbox.height, GRAY);
            DrawRectangleLines(escenario[HABITACION].objetos[FUERATEXTO].hitbox.x, escenario[HABITACION].objetos[FUERATEXTO].hitbox.y, escenario[HABITACION].objetos[FUERATEXTO].hitbox.width, escenario[HABITACION].objetos[FUERATEXTO].hitbox.height, DARKGRAY);
        }
        if( player->subEscenarioActivo == 1 ) {
            DrawRectangleLines(escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.x, escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.y, escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.width, escenario[HABITACION].objetos[FONDOVENTANA1].hitbox.height, RED);
            DrawRectangleLines(escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.x, escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.y, escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.width, escenario[HABITACION].objetos[FONDOVENTANA2].hitbox.height, RED);
            DrawRectangleLines(escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.x, escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.y, escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.width, escenario[HABITACION].objetos[FONDOVENTANA3].hitbox.height, RED);
            DrawRectangleLines(escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.x, escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.y, escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.width, escenario[HABITACION].objetos[FONDOVENTANA4].hitbox.height, RED);
            DrawRectangleLines(escenario[HABITACION].objetos[OREJA].hitbox.x, escenario[HABITACION].objetos[OREJA].hitbox.y, escenario[HABITACION].objetos[OREJA].hitbox.width, escenario[HABITACION].objetos[OREJA].hitbox.height, BLUE);
        }
        if( player->subEscenarioActivo == 2 ) {
            DrawRectangleLines(escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.x, escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.y, escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.width, escenario[HABITACION].objetos[FONDOESPEJO1].hitbox.height, BLUE);
            DrawRectangleLines(escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.x, escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.y, escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.width, escenario[HABITACION].objetos[FONDOESPEJO2].hitbox.height, BLUE);
        }
    }
    if( player->escenario == MENU ) {
        DrawRectangleLines(escenario[MENU].objetos[PLAY].hitbox.x, escenario[MENU].objetos[PLAY].hitbox.y, escenario[MENU].objetos[PLAY].hitbox.width, escenario[MENU].objetos[PLAY].hitbox.height, GREEN);
        DrawRectangleLines(escenario[MENU].objetos[BOTONLOAD].hitbox.x, escenario[MENU].objetos[BOTONLOAD].hitbox.y, escenario[MENU].objetos[BOTONLOAD].hitbox.width, escenario[MENU].objetos[BOTONLOAD].hitbox.height, GREEN);
        DrawRectangleLines(escenario[MENU].objetos[BOTONSCORE].hitbox.x, escenario[MENU].objetos[BOTONSCORE].hitbox.y, escenario[MENU].objetos[BOTONSCORE].hitbox.width, escenario[MENU].objetos[BOTONSCORE].hitbox.height, GREEN);
        DrawRectangleLines(escenario[MENU].objetos[BOTONSAVE].hitbox.x, escenario[MENU].objetos[BOTONSAVE].hitbox.y, escenario[MENU].objetos[BOTONSAVE].hitbox.width, escenario[MENU].objetos[BOTONSAVE].hitbox.height, GREEN);
        DrawRectangleLines(escenario[MENU].objetos[BOTONCREDITS].hitbox.x, escenario[MENU].objetos[BOTONCREDITS].hitbox.y, escenario[MENU].objetos[BOTONCREDITS].hitbox.width, escenario[MENU].objetos[BOTONCREDITS].hitbox.height, GREEN);
    }
    if( player->escenario == GANASTE ) {
        DrawRectangleLines(escenario[GANASTE].objetos[BOTONGANASTE].hitbox.x, escenario[GANASTE].objetos[BOTONGANASTE].hitbox.y, escenario[GANASTE].objetos[BOTONGANASTE].hitbox.width, escenario[GANASTE].objetos[BOTONGANASTE].hitbox.height, GREEN);
    }
    if( player->escenario == CREDITOS ) {
        DrawRectangleLines(escenario[CREDITOS].objetos[BACK].hitbox.x, escenario[CREDITOS].objetos[BACK].hitbox.y, escenario[CREDITOS].objetos[BACK].hitbox.width, escenario[CREDITOS].objetos[BACK].hitbox.height, RED);
    }
    if( player->escenario == SCORE ) {
        DrawRectangleLines(escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.x, escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.y, escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.width, escenario[SCORE].objetos[BOTONSCOREBACK].hitbox.height, GREEN);
    }
//    if( player->escenario == COCINA ) {
//        DrawRectangleLines(escenario[COCINA].objetos[CUADRO].hitbox.x, escenario[COCINA].objetos[CUADRO].hitbox.y, escenario[COCINA].objetos[CUADRO].hitbox.width, escenario[COCINA].objetos[CUADRO].hitbox.height, YELLOW);
//        DrawRectangleLines(escenario[COCINA].objetos[TACHO].hitbox.x, escenario[COCINA].objetos[TACHO].hitbox.y, escenario[COCINA].objetos[TACHO].hitbox.width, escenario[COCINA].objetos[TACHO].hitbox.height, RED);
//        DrawRectangleLines(escenario[COCINA].objetos[HELADERA].hitbox.x, escenario[COCINA].objetos[HELADERA].hitbox.y, escenario[COCINA].objetos[HELADERA].hitbox.width, escenario[COCINA].objetos[HELADERA].hitbox.height, GREEN);
//        DrawRectangleLines(escenario[COCINA].objetos[CUCHILLO].hitbox.x, escenario[COCINA].objetos[CUCHILLO].hitbox.y, escenario[COCINA].objetos[CUCHILLO].hitbox.width, escenario[COCINA].objetos[CUCHILLO].hitbox.height, BLUE);
//        DrawRectangleLines(escenario[COCINA].objetos[PARED].hitbox.x, escenario[COCINA].objetos[PARED].hitbox.y, escenario[COCINA].objetos[PARED].hitbox.width, escenario[COCINA].objetos[PARED].hitbox.height, WHITE);
//        DrawRectangleLines(escenario[COCINA].objetos[MESADA].hitbox.x, escenario[COCINA].objetos[MESADA].hitbox.y, escenario[COCINA].objetos[MESADA].hitbox.width, escenario[COCINA].objetos[MESADA].hitbox.height, VIOLET);
//        DrawRectangleLines(escenario[COCINA].objetos[PUERTA].hitbox.x, escenario[COCINA].objetos[PUERTA].hitbox.y, escenario[COCINA].objetos[PUERTA].hitbox.width, escenario[COCINA].objetos[PUERTA].hitbox.height, ORANGE);
//        DrawRectangleLines(escenario[COCINA].objetos[DENTROTEXTO].hitbox.x, escenario[COCINA].objetos[DENTROTEXTO].hitbox.y, escenario[COCINA].objetos[DENTROTEXTO].hitbox.width, escenario[COCINA].objetos[DENTROTEXTO].hitbox.height, GRAY);
//        DrawRectangleLines(escenario[COCINA].objetos[FUERATEXTO].hitbox.x, escenario[COCINA].objetos[FUERATEXTO].hitbox.y, escenario[COCINA].objetos[FUERATEXTO].hitbox.width, escenario[COCINA].objetos[FUERATEXTO].hitbox.height, DARKGRAY);
//    }

    DrawText(TextFormat("SCORE: %i", player->score * 100), 100, 100, 45, RAYWHITE);
    DrawRectangleLines(herramientaDebug->valorVariable[1] * 100, herramientaDebug->valorVariable[2] * 100, herramientaDebug->valorVariable[3] * 100, herramientaDebug->valorVariable[4] * 100, WHITE);
    ///-------------------------------------------------------------------
}
///Ortiz Rocio, Ortiz Brisa, Olmos Fernando
///TP Programacion y Laboratorio 2, UTN Mar Del Plata 2023
