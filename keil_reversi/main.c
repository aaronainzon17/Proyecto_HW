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
#include "tableros.h"

// Nota: wait es una espera activa. Se puede eliminar poniendo el procesador en modo iddle. Probad a hacerlo

extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

//static volatile struct EventInfo alarma_visualizacion;

static volatile int entradas_anterior = 0x0000FC0;
static volatile int entradas_nuevo;

extern int gestor_SC_in(void);
extern int gestor_SC_out(void);

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

// MAIN 
int main (void) {
	struct EventInfo Evento;
	//int t1,t2;				// Variables de medición de tiempo
	//int tot;
	int PD_Flag = 0;					// Flag para salir de power down
	iniciar();
	while(1){		
		if(cola_nuevos_eventos()){
				cola_leer_evevento_antiguo(&Evento);
				switch(Evento.idEvento){
					case ID_Alarma:	// Alarma pulsacion para comprobar si el boton pulsado lo sigue estando
						sudoku(ID_Alarma);
					break;						
					case ID_EINT1:	// Nueva pulsacion EINT1
						introducir_alarma_power();		// Se resetea la alarma de power down
						Gestor_Pulsacion_Control(Evento.idEvento);	//Pasa a pulsado y pone la alarma; Control pulsacion
						if(PD_Flag == 1){					// Si vienes de power down no haces nada
							PD_Flag = 0;						// SD se pone a 0 para indicar que no estamos en power down
						}else{
							sudoku(ID_EINT1);
						}
					break;
					case ID_EINT2:																			// Nueva pulsacion EINT2
						introducir_alarma_power();													// Se resetea la alarma de power down
						Gestor_Pulsacion_Control(Evento.idEvento);//Cambias a pulsado y pones la alarma de comprobación de pulsación
						if(PD_Flag == 1){					// Si vienes de power down no haces nada
							PD_Flag = 0;						// SD se pone a 0 para indicar que no estamos en power down
						}else{
							sudoku(ID_EINT2);
						}					
					break;
					case ID_Alarma_visualizacion:											// Alarma Visualizacion
						sudoku(ID_Alarma_visualizacion);
					break;
					case ID_mostrar_vis:																			// Mostrar visualizacion
							sudoku_mostrar_visualizacion(Evento);
					break;
					case ID_bit_val:											// Poner el bit de validación a 1 durante 1 segundo
						sudoku(ID_bit_val);
					break;
					case ID_fin_val:						//Acaba la alarma de 1s y ponemos validar a 0 otra vez
						sudoku(ID_fin_val);
					break;
					case ID_power_down:							// Acaba la alarma y entra en powerdown
						gestor_IO_escribir_bit_powerdown();
						PD_Flag = 1;						// Actualizamos el flag de powerdown 
						PM_power_down();
						gestor_IO_apagar_bit_powerdown(); 
					break;
					case ID_timer_0:	//Llega timer
						gestor_alarmas_control_alarma();
					break;
				}
		}else{
			PM_idle();	
		}
	}
}
