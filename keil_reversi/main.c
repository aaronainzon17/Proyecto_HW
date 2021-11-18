#include <LPC210x.H>                       /* LPC210x definitions */
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
#include "gestor_IO.h"

// Nota: wait es una espera activa. Se puede eliminar poniendo el procesador en modo iddle. Probad a hacerlo

extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

static volatile struct EventInfo alarma_visualizacion;

static volatile int entradas_anterior = 0x00000000;
static volatile int entradas_nuevo;

extern int gestor_SC_in(void);
extern int gestor_SC_out(void);


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
 * y devuelve en celdas_vacias el n?mero de celdas vacias
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
 * y devuelve en celdas_vacias el n?mero de celdas vacias
 */
static int candidatos_actualizar_c_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
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
    /* calcula lista de candidatos, versi?n C */
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

// MAIN
int main (void) {
	#include "tableros.h"

	struct EventInfo Evento;
	int fila;
	int columna;
	int valor_celda;
	//uint32_t aux;
	//static volatile struct EventInfo alarma_PD;
	temporizador_iniciar();
	eint0_init();
	gestor_IO_init();
	cola_ini();
	alarma_visualizacion.idEvento = 3;
	alarma_visualizacion.timeStamp = temporizador_leer();
	alarma_visualizacion.auxData = 0x00800014;
	gestor_alarmas_control_cola(alarma_visualizacion);
	while(1){
		//gestor_SC_in();
		//gestor_SC_out();
		/*alarma_PD.idEvento = 3;
		alarma_PD.timeStamp = temporizador_leer();
		aux = (3 & 0xFF000000);
		//1(0x1) porque es periodico y 1000(0X0003E8) de periodo -> (0X8003E8): 24 bits y quedan los 8 de id 
		alarma_PD.auxData = (aux | 0x00000010); // 8 bits de mas peso id de evento, bit 9 si es periodica o no y 23 bits el tiempo de periodo
		gestor_alarmas_control_cola(alarma_PD);*/		
		PM_idle();
		temporizador_periodo(1);
		if(cola_nuevos_eventos()){
				cola_leer_evevento_antiguo(&Evento);
				switch(Evento.idEvento){
					case 0:	// Alarma pulsación
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;						
					case 1:	// Nueva pulsación EINT1
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;
					case 2:	// Nueva pulsación EINT2
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;
					case 3:	// Alarma Visualización
						entradas_nuevo = GPIO_leer(16,12);
						if (entradas_anterior != entradas_nuevo){
							fila = GPIO_leer(16,4);
							columna = GPIO_leer(20,4);
							valor_celda = celda_leer_valor(cuadricula_C_C[fila][columna]);
							GPIO_escribir(0,4,valor_celda);
						}	//Sino no haces nada
					break;
				}
		}
	}
	
	
	
		//int correcto = sudoku9x9(cuadricula_C_C, cuadricula_C_ARM, cuadricula_ARM_ARM, cuadricula_ARM_C, solucion);
		//int i = 0;
		//struct EventInfo Pulsacion_alarma;
		//temporizador_iniciar();
		//temporizador_empezar();
		//eint0_init();
		//GPIO_iniciar();
		//GPIO_marcar_entrada(1,20);
		//GPIO_escribir(8,12,0);
		
		
		/*GPIO_marcar_salida(4,4);
		GPIO_escribir(8,12,10);*/
		
		/*cola_ini();
		for(i = 0; i< 35; i++){
				cola_guardar_eventos(1,i);
		}*/
		//Planificador_Control();
		/*Pulsacion_alarma.idEvento = 1;
		Pulsacion_alarma.timeStamp = temporizador_leer();
		Pulsacion_alarma.auxData = 0x0000210;
		gestor_alarmas_control_cola(Pulsacion_alarma);*/
		/*while(1){
			//eint1_clear_nueva_pulsacion();
			//temporizador_periodo(1);
		}*/
}
