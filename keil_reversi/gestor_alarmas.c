#include <LPC210x.H>                      
#include <inttypes.h>
#include <stddef.h>
#include "Gestor_Alarmas.h"
#include "cola.h"
#include "timer.h"
#include "eventos.h"

static volatile struct EventInfo alarm_queue[8];
static volatile uint8_t bit_activa[8]; 
static volatile uint8_t bit_perio[8];
static volatile uint32_t periodo[8];
static volatile uint32_t periodicas_restaurar[8];

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
		if(periodo_alarma == 0){
			bit_activa[nueva_alarma.idEvento] = 0;
		}else{
			alarm_queue[nueva_alarma.idEvento] = nueva_alarma;
			bit_activa[nueva_alarma.idEvento] = 1;
			bit_perio[nueva_alarma.idEvento] = ((nueva_alarma.auxData & 0x00000800) << 23) & 0x1;
			periodo[nueva_alarma.idEvento] = periodo_alarma;
			periodicas_restaurar[nueva_alarma.idEvento] = periodo_alarma;
		}
	}
}

void gestor_alarmas_control_alarma(void){
	int i = 0;
	while (i<8){
		if(bit_activa[i] != 0){
			periodo[i] --;
			if(periodo[i] == 0){
					// Deshabilitas interr.
					/*VICIntEnClr |= 0xFFFFFFFF			// Inhabilitamos las interrupciones
					VICIntEnable &= ~255;*/
					cola_guardar_eventos(alarm_queue[i].idEvento, alarm_queue[i].auxData);
					//Habilitas interr.
					if(bit_perio[i] != 1){
						bit_activa[i] = 0;
					}
					else{
						periodo[i] = periodicas_restaurar[i];
					}
			}
		}
		i++;
	}
}




