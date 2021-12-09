#include <LPC210x.H>                      
#include <inttypes.h>
#include <stddef.h>
#include "Gestor_Alarmas.h"
#include "cola.h"
#include "timer.h"
#include "eventos.h"

static volatile struct EventInfo alarm_queue[13];  	// Vector de alarmas de eventos; cada índice corresponde a un tipo de alarma
static volatile uint8_t bit_activa[13];				// Vector que indica si la alarma de cada tipo esta activa o no
static volatile uint8_t bit_perio[13]; 				// Vector que indica si la alarma de cada tipo es periódica o no
static volatile uint32_t periodo[13]; 				// Vector que indica el periodo de cada tipo de alarma
static volatile uint32_t periodicas_restaurar[13];	// Vector que almacena el periodo original de cada alarma

// Inicializar las alarmas
void iniciar_alarmas(void){
	int i=0;
	while (i<13){
		bit_activa[i] = 0;
		i++;
	}
}
/*
* Cuando se introduce una nueva alarma actualiza todos los vectores de ese tipo de alarma con
* los datos de la alarma introducida
*/
void gestor_alarmas_control_cola(struct EventInfo nueva_alarma){
	uint32_t periodo_alarma = nueva_alarma.auxData  & 0x007FFFFF; // Campo en el que se almacena el periodo de la alarma
	if(bit_activa[nueva_alarma.idEvento] == 0){
		if(periodo_alarma != 0){
			alarm_queue[nueva_alarma.idEvento] = nueva_alarma;
			bit_activa[nueva_alarma.idEvento] = 1;
			bit_perio[nueva_alarma.idEvento] = ((nueva_alarma.auxData & 0x00800000) >> 23); // Bit 9 de mayor peso para saber si es periodico
			periodo[nueva_alarma.idEvento] = periodo_alarma;
			periodicas_restaurar[nueva_alarma.idEvento] = periodo_alarma;
		}
	}else{
		if(periodo_alarma == 0){		// si el periodo es 0 quitas la alarma por lo que pones el vector de activas a 0
			bit_activa[nueva_alarma.idEvento] = 0;
		}else{	// Si no es que quieres actualizar la alarma y establecer un nuevo periodo
			alarm_queue[nueva_alarma.idEvento] = nueva_alarma;
			bit_activa[nueva_alarma.idEvento] = 1;
			bit_perio[nueva_alarma.idEvento] = ((nueva_alarma.auxData & 0x00000800) << 23) & 0x1;
			periodo[nueva_alarma.idEvento] = periodo_alarma;
			periodicas_restaurar[nueva_alarma.idEvento] = periodo_alarma;
		}
	}
}

/*
* En cada interrupcion del timer resta 1 a los periodos de las alarmas para asi 
* finalizarlos y determinar cuando acaba la alarma 
*/
void gestor_alarmas_control_alarma(void){
	int i = 0;
	while (i<13){
		if(bit_activa[i] != 0){	// Si la alarma esta activa
			periodo[i] --;
			if(periodo[i] == 0){ // Si el periodo es 0 significa que ha acabado la alarma 
				//Se encola un evento del tipo de esa alarma  
				cola_guardar_eventos(alarm_queue[i].idEvento, alarm_queue[i].auxData);
				if(bit_perio[i] != 1){	// si la alarma no es periódica pones el bit de activa a 0 y acaba
					bit_activa[i] = 0;
				}
				else{
					periodo[i] = periodicas_restaurar[i];	// Sino reseteas el periodo al inicial y vuelve a empezar la alarma
				}
			}
		}
		i++;
	}
}

void introducir_alarma_power(void){
	struct EventInfo Power_down;
	Power_down.idEvento = ID_power_down;
	//Power_down.timeStamp = temporizador_leer();
	Power_down.auxData = 0x0000AFC8;				// Ponemos la alarma 15 segundos para el powerdown
	gestor_alarmas_control_cola(Power_down);
}

void introducir_alarma_viualizacion(void){
	struct EventInfo alarma_visualizacion;
	alarma_visualizacion.idEvento = ID_Alarma_visualizacion;
	//alarma_visualizacion.timeStamp = temporizador_leer();
	alarma_visualizacion.auxData = 0x00800014;				// Ponemos la alarma 5 veces por segundo periodica para la visualizacion
	gestor_alarmas_control_cola(alarma_visualizacion);		// La introducimos al gestor de alarmas
}

void gestor_alarma_visualizacion_1s(void){
	struct EventInfo led_alrm;	// Evento que se genera cuando se ha introducido una entradavalida y se activa el bit 13 de la GPIO durante 1 s
	led_alrm.idEvento = ID_fin_val;						
	//led_alrm.timeStamp = temporizador_leer();
	led_alrm.auxData = 0x000000A4;				// Ponemos la alarma  de 1 segundo
	gestor_alarmas_control_cola(led_alrm);
}

void introducir_alarma_iddle(void){
	struct EventInfo iddle_alarm;
	iddle_alarm.idEvento = ID_iddle;
	iddle_alarm.auxData = 0x00800030;
	gestor_alarmas_control_cola(iddle_alarm);
}

void apagar_alarma_iddle(void){
	struct EventInfo iddle_alarm;
	iddle_alarm.idEvento = ID_iddle;
	iddle_alarm.auxData = 0x00000000;
	gestor_alarmas_control_cola(iddle_alarm);
}


