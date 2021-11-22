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

// Nota: wait es una espera activa. Se puede eliminar poniendo el procesador en modo iddle. Probad a hacerlo

extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

static volatile struct EventInfo alarma_visualizacion;

static volatile int entradas_anterior = 0x0000FC0;
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

void introducir_alarma_power(void){
	struct EventInfo Power_down;
	Power_down.idEvento = 7;
	Power_down.timeStamp = temporizador_leer();
	Power_down.auxData = 0x0000AFC8;	// Ponemos la alarma 5 veces por segundo periódica para la visualización
	gestor_alarmas_control_cola(Power_down);
}

// MAIN
int main (void) {
	#include "tableros.h"

	struct EventInfo Evento;
	struct EventInfo Most_Vis;
	struct EventInfo led_val;
	struct EventInfo led_alrm;
	int fila;
	int columna;
	int valor_celda;
	int nuevo_valor;
	int candidatos;
	int j,k;
	temporizador_iniciar();
	eint0_init();
	gestor_IO_init();
	cola_ini();
	candidatos_actualizar_c(cuadricula_C_C);
	temporizador_periodo(1);
	alarma_visualizacion.idEvento = 3;
	alarma_visualizacion.timeStamp = temporizador_leer();
	alarma_visualizacion.auxData = 0x00800014;	// Ponemos la alarma 5 veces por segundo periódica para la visualización
	gestor_alarmas_control_cola(alarma_visualizacion);
	introducir_alarma_power();
	while(1){	
		PM_idle();
		if(cola_nuevos_eventos()){
				cola_leer_evevento_antiguo(&Evento);
				switch(Evento.idEvento){
					case 0:	// Alarma pulsación
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;						
					case 1:	// Nueva pulsación EINT1
						introducir_alarma_power();
						Gestor_Pulsacion_Control(Evento.idEvento);
						fila = GPIO_leer(16,4);
						columna = GPIO_leer(20,4);
						if((((cuadricula_C_C[fila][columna] >> 7) & 0x00000001) != 0x00000001) || (fila == 0 && columna == 0 && nuevo_valor ==0)){
							nuevo_valor = GPIO_leer(24,4);
							if(fila == 0 && columna == 0 && nuevo_valor ==0){
								//Reiniciamos el juego
								for(j=0;j<9;j++){
									for(k=0;k<16;k++){
										cuadricula_C_C[j][k] = cuadricula[j][k];
									}
								}
								candidatos_actualizar_c(cuadricula_C_C);
								//Poner en modo iddle
							}else{
								cuadricula_C_C[fila][columna]&= 0xFFFFFFF0;
								cuadricula_C_C[fila][columna] += nuevo_valor;	//
								//Iniciar tiempo
								candidatos_propagar_c(cuadricula_C_C,fila,columna);
								//Parar tiempo
							}
						}			
					break;
					case 2:	// Nueva pulsación EINT2
						introducir_alarma_power();
						Gestor_Pulsacion_Control(Evento.idEvento);
						fila = GPIO_leer(16,4);
						columna = GPIO_leer(20,4);
						if(((cuadricula_C_C[fila][columna] >> 7) & 0x00000001) != 0x00000001){
							cuadricula_C_C[fila][columna] = 0x00000000;	//
							candidatos_actualizar_c(cuadricula_C_C);
						}
					break;
					case 3:	// Alarma Visualización
						entradas_nuevo = GPIO_leer(16,12);
						if (entradas_anterior != entradas_nuevo){
							introducir_alarma_power();
							Most_Vis.idEvento = 4;
							Most_Vis.auxData = entradas_nuevo;	
							cola_guardar_eventos(Most_Vis.idEvento,Most_Vis.auxData);						
						}	//Sino no haces nada
					break;
					case 4:	// Mostrar visualización
							fila = GPIO_leer(16,4);
							columna = GPIO_leer(20,4);
							if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) == 0x00000001){
								GPIO_escribir(13,1,1); //Bit de validar permanente
							}else{
								GPIO_escribir(13,1,0);
								led_val.idEvento = 5;
								cola_guardar_eventos(led_val.idEvento,led_val.auxData);
							}
							valor_celda = celda_leer_valor(cuadricula_C_C[fila][columna]);
							GPIO_escribir(0,4,valor_celda);
							candidatos = ((cuadricula_C_C[fila][columna] >> 7));	//
							candidatos = ~candidatos;
							candidatos = candidatos & 0x1FF;
							GPIO_escribir(4,9,candidatos);
							entradas_anterior = Evento.auxData;
					break;
					case 5:
						//Ponemos alarma de 1 s para bit de validar
						GPIO_escribir(13,1,1);
						led_alrm.idEvento = 6;
						led_alrm.timeStamp = temporizador_leer();
						led_alrm.auxData = 0x00000064;	// Ponemos la alarma 5 veces por segundo periódica para la visualización
						gestor_alarmas_control_cola(led_alrm);
					break;
					case 6:	//Acaba la alarma de 1s y ponemos validar a 0 otra vez
						GPIO_escribir(13,1,0);
					break;
					case 7:	// Acaba la alarma y entra en powerdown
						PM_power_down();
					break;
				}
		}
	}
}
