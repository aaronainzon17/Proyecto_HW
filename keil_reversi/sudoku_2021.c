#include <LPC210x.H>                      
#include <inttypes.h>
#include <stddef.h>
#include "eventos.h"
#include "gestor_IO.h"
#include "timer0.h"
#include "Power_management.h"
#include "boton_eint0.h"
#include <stddef.h>
#include "sudoku_2021.h"
#include "gpio.h"
#include "boton_eint0.h"
#include "gestor_alarmas.h"
#include "cola.h"
#include "planificador.h"
#include "gestor_pulsacion.h"
#include "eventos.h"
#include "tableros.h"



extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

static volatile int entradas_anterior = 0x0000FC0;
static volatile int entradas_nuevo;

static volatile CELDA
cuadricula[NUM_FILAS][NUM_COLUMNAS] =
{
0x0015, 0x0000, 0x0000, 0x0013, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0000, 0x0000, 0x0019, 0x0000, 0x0000, 0x0000, 0x0015, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0019, 0x0016, 0x0017, 0x0000, 0x0015, 0x0000, 0x0013, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0018, 0x0000, 0x0019, 0x0000, 0x0000, 0x0016, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0015, 0x0018, 0x0016, 0x0011, 0x0014, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0014, 0x0012, 0x0000, 0x0013, 0x0000, 0x0017, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0017, 0x0000, 0x0015, 0x0000, 0x0019, 0x0012, 0x0016, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0016, 0x0000, 0x0000, 0x0000, 0x0018, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0012, 0x0000, 0x0000, 0x0011, 0, 0, 0, 0, 0, 0, 0
};


/* *****************************************************************************
 * propaga el valor de una determinada celda en C
 * para actualizar las listas de candidatos
 * de las celdas en su su fila, columna y región */
/* Recibe como parametro la cuadricula, y la fila y columna de
 * la celda a propagar; no devuelve nada
 */
void candidatos_propagar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
	uint8_t fila, uint8_t columna)
{
    uint8_t j, i , init_i, init_j, end_i, end_j;
    /* puede ayudar esta "look up table" a mejorar el rendimiento */
    const uint8_t init_region[NUM_FILAS] = {0, 0, 0, 3, 3, 3, 6, 6, 6};

    /* valor que se propaga */
    uint8_t valor = celda_leer_valor(cuadricula[fila][columna]);

    /* recorrer fila descartando valor de listas candidatos */
    for (j=0;j<NUM_FILAS;j++)
	celda_eliminar_candidato(&cuadricula[fila][j],valor);

    /* recorrer columna descartando valor de listas candidatos */
    for (i=0;i<NUM_FILAS;i++)
	celda_eliminar_candidato(&cuadricula[i][columna],valor);

    /* determinar fronteras región */
    init_i = init_region[fila];
    init_j = init_region[columna];
    end_i = init_i + 3;
    end_j = init_j + 3;

    /* recorrer region descartando valor de listas candidatos */
    for (i=init_i; i<end_i; i++) {
      for(j=init_j; j<end_j; j++) {
	      celda_eliminar_candidato(&cuadricula[i][j],valor);
	    }
    }
}

/* *****************************************************************************
 * calcula todas las listas de candidatos (9x9)
 * necesario tras borrar o cambiar un valor (listas corrompidas)
 * retorna el numero de celdas vacias */

/* Init del sudoku en codigo C invocando a propagar en C
 * Recibe la cuadricula como primer parametro
 * y devuelve en celdas_vacias el n�mero de celdas vacias
 */
static int candidatos_actualizar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
    int celdas_vacias = 0;
    uint8_t i,j;

	//borrar todos los candidatos
    for(i = 0; i < NUM_FILAS; i++) {
        for(j = 0; j < NUM_FILAS; j++) {
            celda_establecer_todos_candidatos(&cuadricula[i][j]); 
        }
    }
	//recalcular candidatos de las celdas vacias calculando cuantas hay vacias
    for(i = 0; i < NUM_FILAS; ++i) {
        for(j = 0; j < NUM_FILAS; ++j) {
            if (celda_leer_valor(cuadricula[i][j]) == 0x0){
                celdas_vacias++;
            }else{
                //candidatos_propagar_arm(cuadricula,i,j);
                candidatos_propagar_c(cuadricula,i,j);
            }
        }
    }

	//retornar el numero de celdas vacias
	return celdas_vacias; // por favor eliminar una vez completada la función
}

/* Init del sudoku en codigo C invocando a propagar en arm
 * Recibe la cuadricula como primer parametro
 * y devuelve en celdas_vacias el n�mero de celdas vacias
 */
int candidatos_actualizar_c_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{	
	int celdas_vacias = 0;
    uint8_t i,j;

	//borrar todos los candidatos
    for(i = 0; i < NUM_FILAS; i++) {
        for(j = 0; j < NUM_FILAS; j++) {
            celda_establecer_todos_candidatos(&cuadricula[i][j]); 
        }
    }
	//recalcular candidatos de las celdas vacias calculando cuantas hay vacias
    for(i = 0; i < NUM_FILAS; ++i) {
        for(j = 0; j < NUM_FILAS; ++j) {
            if (celda_leer_valor(cuadricula[i][j]) == 0x0){
                celdas_vacias++;
            }else{
                candidatos_propagar_arm(cuadricula,i,j);
                //candidatos_propagar_c(cuadricula,i,j);
            }
        }
    }

	//retornar el numero de celdas vacias
	return celdas_vacias; // por favor eliminar una vez completada la función
	
	//return candidatos_actualizar_arm_c(cuadricula); // por favor eliminar una vez completada la función
}

static int cuadricula_candidatos_verificar(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
	 CELDA solucion[NUM_FILAS][NUM_COLUMNAS])
{
    int correcto = 1;
    uint8_t i;
    uint8_t j;

    for(i=0; i < NUM_FILAS && 1 == correcto; ++i) {
       for(j=0; j < NUM_FILAS && 1 == correcto; ++j) {
	   correcto = cuadricula[i][j] == solucion[i][j];
       }
    }
    return correcto;
}

/* ************************************************************************
 * programa principal del juego que recibe el tablero
 */
int
sudoku9x9(CELDA cuadricula_C_C[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_C_ARM[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_ARM_ARM[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_ARM_C[NUM_FILAS][NUM_COLUMNAS],
	CELDA solucion[NUM_FILAS][NUM_COLUMNAS])
{
    int celdas_vacias[4];     //numero de celdas aun vacias
    int correcto = 0;
	  size_t i;
    /* calcula lista de candidatos, versi�n C */
    celdas_vacias[0] = candidatos_actualizar_c(cuadricula_C_C);

    //    /* Init C con propagar arm */
    celdas_vacias[1] = candidatos_actualizar_c_arm(cuadricula_C_ARM);

    //    /* Init arm con propagar arm */
    celdas_vacias[2] = candidatos_actualizar_arm_arm(cuadricula_ARM_ARM);

    //    /* Init arm con propagar c */
    celdas_vacias[3] = candidatos_actualizar_arm_c(cuadricula_ARM_C);
	
	  for (i=1; i < 4; ++i) {
			if (celdas_vacias[i] != celdas_vacias[0]) {
				return -1;
			}
		}

    /* verificar que la lista de candidatos C_C calculada es correcta */
    correcto = cuadricula_candidatos_verificar(cuadricula_C_C,solucion);
    correcto += cuadricula_candidatos_verificar(cuadricula_ARM_ARM,solucion);
    correcto += cuadricula_candidatos_verificar(cuadricula_C_ARM,solucion);
    correcto += cuadricula_candidatos_verificar(cuadricula_ARM_C,solucion);
    return correcto;
}

void sudoku_validacion_1s (void){
		gestor_IO_escribir_en_gpio(13,1,1);
		gestor_alarma_visualizacion_1s();
}

void sudoku_jugada (void){
//		struct EventInfo led_val;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
		int fila;
		int columna;
		int nuevo_valor;
		int j,k, guarda;
		fila = gestor_IO_leer_de_gpio(16,4);
		columna = gestor_IO_leer_de_gpio(20,4);
		nuevo_valor = gestor_IO_leer_de_gpio(24,4);
		// La casilla introducida no es pista y no se ha introducido fila = 0, columna = 0, valor = 0
		if((((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001) || (fila == 0 && columna == 0 && nuevo_valor ==0)){	
			if(fila == 0 && columna == 0 && nuevo_valor ==0){
				//Reiniciamos el juego
				for(j=0;j<9;j++){
					for(k=0;k<16;k++){
						cuadricula_C_C[j][k] = cuadricula[j][k];	// Reiniciamos cada casilla
					}
				}
				candidatos_actualizar_c(cuadricula_C_C);	// Actualizamos candidatos
			}else{	// Se puede escribir en esa casilla porque no es pista
				guarda = (cuadricula_C_C[fila][columna] >> 6);	
				guarda = guarda >> nuevo_valor;	// Compruebas que el valor a introducir esta como candidato
				if( (guarda & 0x00000001) == 0 ){	// Si el bit esta a uno puedes escribir
					cuadricula_C_C[fila][columna]&= 0xFFFFFFF0;
					cuadricula_C_C[fila][columna] += nuevo_valor;	// Escribes el nuevo valor
					//Iniciar tiempo
					//t1 = temporizador_leer();
					candidatos_propagar_c(cuadricula_C_C,fila,columna);	// Propagas el nuevo valor
					//Parar tiempo
					//t2 = temporizador_leer();
					//tot = t2-t1;	//Tiempo total
					//led_val.idEvento = ID_bit_val;	// Poner el bit de validación a 1 durante 1 segundo mediante la generación de 1 evento
					//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
					//cola_guardar_eventos(led_val.idEvento,led_val.auxData);
					sudoku_validacion_1s();
					
				}
			}
		}
}

void sudoku_jugada_borrar(void){
//	struct EventInfo led_val2;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
	int fila;
	int columna;
	fila = gestor_IO_leer_de_gpio(16,4);
	columna = gestor_IO_leer_de_gpio(20,4);
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001){	// Comprobamos que la casilla no es pista por lo que no se podría borrar
		cuadricula_C_C[fila][columna] = 0x00000000;							// Eliminamos el valor de la casilla
		candidatos_actualizar_c(cuadricula_C_C);							// Actualizamos los candidatos
		//led_val2.idEvento = ID_bit_val;												// Poner el bit de validación a 1 durante 1 segundo mediante la generación de 1 evento
		//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
		//cola_guardar_eventos(led_val2.idEvento,led_val2.auxData);
		sudoku_validacion_1s();
	}
}

void sudoku_programar_visualizacion(void){
	struct EventInfo Most_Vis;	// Evento que se genera para mostrarla visualización una vez a habido cambios en la entrada
	entradas_nuevo = GPIO_leer(16,12);			// Lees las entradas de la GPIO
	if (entradas_anterior != entradas_nuevo){	// Si la entrada ha cambiado
		introducir_alarma_power();				// Reseteas la alarma de power_down
		Most_Vis.idEvento = ID_mostrar_vis;					// Generamos evento de visualización si ha cambiado la entrada para actualizar los candidatos y el valor
		Most_Vis.auxData = entradas_nuevo;	
		//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
		cola_guardar_eventos(Most_Vis.idEvento,Most_Vis.auxData);						
	}	//Sino no haces nada
}

void sudoku (int Evento){
    switch(Evento){
					case ID_Alarma:	
						Gestor_Pulsacion_Control(Evento);
					break;
					case ID_EINT1:						
						sudoku_jugada();
					break;
					case ID_EINT2:	
						sudoku_jugada_borrar();
					break;
					case ID_Alarma_visualizacion:	
						sudoku_programar_visualizacion();
					break;
					case ID_bit_val:	
						sudoku_validacion_1s();
					break;
					case ID_fin_val:	
						gestor_IO_escribir_bit_validar();
					break;
		}		
}

void sudoku_mostrar_visualizacion(struct EventInfo Evento){
	int fila;
	int columna;
	int valor_celda;
	int candidatos;
	fila = gestor_IO_leer_de_gpio(16,4); 												// Se lee la fila de la GPIO
	columna = gestor_IO_leer_de_gpio(20,4);												// Se lee la fila de la GPIO
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) == 0x00000001){	// Si la casilla es una pista enciende el bit de validar hasta que se deselecciona
		gestor_IO_escribir_en_gpio(13,1,1); 												//Bit de validar permanente
	}else{
		gestor_IO_escribir_en_gpio(13,1,0);												// Sino quitamos el bit de validar permanente
	}
	valor_celda = celda_leer_valor(cuadricula_C_C[fila][columna]);			// Sacamos el valor de la celda
	GPIO_escribir(0,4,valor_celda);											// Escribimos el valor de la celda
	candidatos = ((cuadricula_C_C[fila][columna] >> 7));					// calculamos los candidatos
	candidatos = ~candidatos;
	candidatos = candidatos & 0x1FF;
	gestor_IO_escribir_en_gpio(4,9,candidatos);											// Escribimos los candidatos
	entradas_anterior = Evento.auxData;										// Actualizamos la entrada para la siguiente pasada
}

void sudoku_iniciar_tablero(void){
	candidatos_actualizar_c(cuadricula_C_C);
}


