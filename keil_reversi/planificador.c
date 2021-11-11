#include <LPC210X.H>                            // LPC21XX Peripheral Registers
#include "cola.h"
#include "planificador.h"
#include "gestor_pulsacion.h"
#include "gestor_alarmas.h"
#include "Power_management.h"
#include "eventos.h"
#include "timer0.h"
#include "boton_eint0.h"
#include "gpio.h"


void Planificador_Control(void){
	struct EventInfo Evento;
	//uint32_t aux;
	//static volatile struct EventInfo alarma_PD;
	temporizador_iniciar();
	eint0_init();
	GPIO_iniciar();
	cola_ini();
	while(1){
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
					case 0:
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;						
					case 1:
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;
					case 2:
						Gestor_Pulsacion_Control(Evento.idEvento);
					break;
				}
		}	
	}
	
}
