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
#include "gestor_IO.h"

static volatile struct EventInfo alarma_visualizacion;

static volatile int entradas_anterior = 0x00000000;
static volatile int entradas_nuevo;

extern int gestor_SC_in(void);
extern int gestor_SC_out(void);

void Planificador_Control(void){
	struct EventInfo Evento;
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
							// Nueva visualización
						}	//Sino no haces nada
					break;
				}
		}	
	}
	
}
