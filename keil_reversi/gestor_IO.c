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

void gestor_IO_nueva_jugada(void){
	struct EventInfo led_val;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
	int fila;
	int columna;
	//int valor_celda;
	int nuevo_valor;
	int j,k, guarda;
	fila = GPIO_leer(16,4);
	columna = GPIO_leer(20,4);
	nuevo_valor = GPIO_leer(24,4);
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
				led_val.idEvento = ID_bit_val;	// Poner el bit de validación a 1 durante 1 segundo mediante la generación de 1 evento
				//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
				cola_guardar_eventos(led_val.idEvento,led_val.auxData);
			}
		}
	}
}

void gestor_IO_jugada_de_borrar(void){
	struct EventInfo led_val2;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
	int fila;
	int columna;
	fila = GPIO_leer(16,4);
	columna = GPIO_leer(20,4);
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001){	// Comprobamos que la casilla no es pista por lo que no se podría borrar
		cuadricula_C_C[fila][columna] = 0x00000000;							// Eliminamos el valor de la casilla
		candidatos_actualizar_c(cuadricula_C_C);							// Actualizamos los candidatos
		led_val2.idEvento = ID_bit_val;												// Poner el bit de validación a 1 durante 1 segundo mediante la generación de 1 evento
		//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
		cola_guardar_eventos(led_val2.idEvento,led_val2.auxData);
	}
}

void gestor_IO_visualizacion(void){
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

void gestor_IO_mostrar_visualizacion(struct EventInfo Evento){
	int fila;
	int columna;
	int valor_celda;
	int candidatos;
	fila = GPIO_leer(16,4); 												// Se lee la fila de la GPIO
	columna = GPIO_leer(20,4);												// Se lee la fila de la GPIO
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) == 0x00000001){	// Si la casilla es una pista enciende el bit de validar hasta que se deselecciona
		GPIO_escribir(13,1,1); 												//Bit de validar permanente
	}else{
		GPIO_escribir(13,1,0);												// Sino quitamos el bit de validar permanente
	}
	valor_celda = celda_leer_valor(cuadricula_C_C[fila][columna]);			// Sacamos el valor de la celda
	GPIO_escribir(0,4,valor_celda);											// Escribimos el valor de la celda
	candidatos = ((cuadricula_C_C[fila][columna] >> 7));					// calculamos los candidatos
	candidatos = ~candidatos;
	candidatos = candidatos & 0x1FF;
	GPIO_escribir(4,9,candidatos);											// Escribimos los candidatos
	entradas_anterior = Evento.auxData;										// Actualizamos la entrada para la siguiente pasada
}

void iniciar(void){
	temporizador_iniciar();									// Inicializamos timers
	temporizador_empezar();
	eint0_init();											// Inicializamos botones
	gestor_IO_init();										// Inicializamos GPIO
	cola_ini();												// Inicializamos cola
	sudoku_iniciar_tablero();
	candidatos_actualizar_c(cuadricula_C_C); 				// Se actualizan los candidatos de cada celda
	temporizador_periodo(1);								// Se configura el timer para que salte cada 1ms
	introducir_alarma_viualizacion();
	introducir_alarma_power();								// Introducimos la alarma de 15 s para el powerdown
} 

void gestor_IO_validacion_1s(void){
		struct EventInfo led_alrm;	// Evento que se genera cuando se ha introducido una entradavalida y se activa el bit 13 de la GPIO durante 1 s
		GPIO_escribir(13,1,1);
		led_alrm.idEvento = ID_fin_val;						
		led_alrm.timeStamp = temporizador_leer();
		led_alrm.auxData = 0x00000064;				// Ponemos la alarma  de 1 segundo
		gestor_alarmas_control_cola(led_alrm);
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

