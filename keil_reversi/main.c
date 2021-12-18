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
#include "SWI_functions.h"
#include "gestor_UART.h"
#include "funciones_escritura.h"
#include "serial_port.h"
#include "gestor_WD.h"


// Nota: wait es una espera activa. Se puede eliminar poniendo el procesador en modo iddle. Probad a hacerlo

extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

//static volatile struct EventInfo alarma_visualizacion;

extern int gestor_SC_in(void);
extern int gestor_SC_out(void);



// MAIN 
int main (void) {
	struct EventInfo Evento;
	//int t1,t2;				// Variables de medición de tiempo
	//int tot;
	int PD_Flag = 0;					// Flag para salir de power down
	int Reset = 0;
	int Parpadeo=0;
	uint32_t volatile buffer;
	int iddle_bit_flag = 0;
	iniciar();
	WT_init(15);
	while(1){	
		WD_feed();
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
							if(Reset==0){
								Reset=1;
								sudoku_jugar();
							}else{
								sudoku(ID_EINT1);}
						}
					break;
					case ID_EINT2:																			// Nueva pulsacion EINT2
						introducir_alarma_power();													// Se resetea la alarma de power down
						Gestor_Pulsacion_Control(Evento.idEvento);//Cambias a pulsado y pones la alarma de comprobación de pulsación
						if(PD_Flag == 1){					// Si vienes de power down no haces nada
							PD_Flag = 0;						// SD se pone a 0 para indicar que no estamos en power down
						}else{
							if(Reset==0){
								Reset=1;
								sudoku_jugar();
							}else{
								sudoku(ID_EINT2);}
						}					
					break;
					case ID_Alarma_visualizacion:											// Alarma Visualizacion
						sudoku(ID_Alarma_visualizacion);
					break;
					case ID_mostrar_vis:																			// Mostrar visualizacion
							sudoku_mostrar_visualizacion(Evento);
					break;
					case ID_bit_val:											// Poner el bit de validación a 1 durante 1 segundo; Jugada exitosa
						sudoku(ID_bit_val);
					break;
					case ID_fin_val:						//Acaba la alarma de 1s y ponemos validar a 0 otra vez; Fin bit
						sudoku(ID_fin_val);
					break;
					case ID_power_down:							// Acaba la alarma y entra en powerdown		
						apagar_alarma_iddle();
						PD_Flag = 1;						// Actualizamos el flag de powerdown 
						PM_power_down(); 
						introducir_alarma_iddle();
					break;
					case ID_timer_0:	//Llega timer
						gestor_alarmas_control_alarma();
					break;
					case ID_iddle:
						if(iddle_bit_flag == 0){
							gestor_IO_escribir_bit_powerdown();
							iddle_bit_flag = 1;
						}else{
							gestor_IO_apagar_bit_powerdown();
							iddle_bit_flag = 0;
						}
					break;
					case ID_UART0:
						introducir_alarma_power();
						gestor_UART(Evento.auxData);
					break;
					case ID_RST:
						Reset=0;
						sudoku_reset();
					break;
					case ID_NEW:
						Reset=1;
						sudoku_jugar();
					break;
					case ID_Evento_RDY:
						resume_write();
					break;
					case ID_ESPERAR_CONFIRMACION:
						buffer=Evento.auxData;
						sudoku_mostrar_vista_previa(buffer);
						introducir_alarma_aceptar();
						introducir_alarma_parpadeo_aceptar();
					break;
					case ID_FIN_ACEPTAR	:
						quitar_alarma_parpadeo_aceptar();
						sudoku_jugada_UART(buffer);
					break;
					case ID_FINAL_JUEGO	:
						Reset=0;
						sudoku_reset();
					break;
					case ID_PARPADEO_ACEPTAR	:
						if(Parpadeo==0){Parpadeo=1;gestor_IO_escribir_en_gpio(13,1,1);}else{Parpadeo=0;gestor_IO_escribir_en_gpio(13,1,0);}
						
					break;
				}
		}else{
			PM_idle();	
		}
	}
}
