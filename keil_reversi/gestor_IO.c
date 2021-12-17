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
//#include "gestor_IO.h"
#include "tableros.h"
#include "serial_port.h"
#include "gestor_RTC.h"

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

void gestor_IO_init(void){	// Inicia la GPIO y marca los pines que van a ser entradas 
	GPIO_iniciar();
	GPIO_marcar_entrada(14,14);
}

void iniciar(void){
	temporizador_iniciar();									// Inicializamos timers
	temporizador_empezar();
	RTC_init();
	eint0_init();											// Inicializamos botones
	gestor_IO_init();										// Inicializamos GPIO
	cola_ini();												// Inicializamos cola
	init_serial();
	sudoku_iniciar_tablero();
	candidatos_actualizar_c(cuadricula_C_C); 				// Se actualizan los candidatos de cada celda
	temporizador_periodo(1);								// Se configura el timer para que salte cada 1ms
	introducir_alarma_viualizacion();
	introducir_alarma_power();								// Introducimos la alarma de 15 s para el powerdown
	introducir_alarma_iddle();
	sudoku_inicio();
} 

void gestor_IO_escribir_bit_validar(void){
	GPIO_escribir(13,1,0);	// Cuando llega la alarma quitas el bit de validar
}

void gestor_IO_escribir_bit_powerdown(void){
	GPIO_escribir(31,1,1);		// ponemos a 1 el bit 31 de la GPIO para indicar que estamos en power down
}

void gestor_IO_apagar_bit_powerdown(void){
	GPIO_escribir(31,1,0);		// ponemos a 1 el bit 31 de la GPIO para indicar que estamos en power down
}

int gestor_IO_leer_de_gpio(int inicio, int fin){
	return GPIO_leer(inicio,fin);
}

void gestor_IO_escribir_en_gpio(int inicio, int fin, int valor){
	GPIO_escribir(inicio,fin,valor);
}

