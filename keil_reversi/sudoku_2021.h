#include <LPC210x.H>                      
#include <inttypes.h>
#include "celda.h"
#include "eventos.h"

void sudoku (int Evento);

/* Tama�os de la cuadricula */
/* Se utilizan 16 columnas para facilitar la visualizaci�n */
enum {NUM_FILAS = 9,
      PADDING = 7,
      NUM_COLUMNAS = NUM_FILAS + PADDING};

/* Definiciones para valores muy utilizados */
enum {FALSE = 0, TRUE = 1};

typedef uint16_t CELDA;
/* La informaci�n de cada celda est� codificada en 16 bits
 * con el siguiente formato, empezando en el bit m�s significativo (MSB):
 * 1 bit  PISTA
 * 1 bit  ERROR
 * 1 bit  no usado
 * 9 bits vector CANDIDATOS (0: es candidato, 1: no es candidato)
 * 4 bits VALOR
 */

/* declaracion de funciones visibles en el exterior */
int sudoku9x9(CELDA cuadricula_C_C[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_C_ARM[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_ARM_ARM[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_ARM_C[NUM_FILAS][NUM_COLUMNAS],
	CELDA solucion[NUM_FILAS][NUM_COLUMNAS]);

void
candidatos_propagar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
                             uint8_t fila, uint8_t columna);
int candidatos_actualizar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

/* declaracion de funciones ARM a implementar */
int
candidatos_actualizar_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

int
candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

int
candidatos_propagar_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
                               uint8_t fila, uint8_t columna);

void sudoku_mostrar_visualizacion(struct EventInfo Evento);

void sudoku_iniciar_tablero(void);

void sudoku_reset (void);

void sudoku_jugada_UART (int auxdata);

void mostrar_tablero(void);

void mostrar_candidatos(void);

void sudoku_inicio(void);

void sudoku_jugar(void);

void sudoku_mostrar_vista_previa(int buffer);

void sudoku_nueva_partida(void);

int sudoku_numero_candidatos(int fila, int columna);

void sudoku_restaurar_estado(void);
	
