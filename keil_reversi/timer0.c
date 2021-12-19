
#include <LPC210X.H>                            // LPC21XX Peripheral Registers
#include "timer0.h"
#include "pulsacion.h"
#include "gestor_alarmas.h"
#include "eventos.h"
#include "cola.h"
// variable para contabilizar el número de interrupciones
static volatile unsigned int timer0_int_count = 0;
static volatile unsigned int timer1_int_count = 0;
static volatile unsigned int TC_init = 0; 								//Ticks al empezar
static volatile unsigned int periodo_alarm = 0; 					//Periodo del temporizador
static volatile unsigned int periodo_alarma2 = 0; 					//Periodo del temporizador
static volatile unsigned int siguiente_periodo = 0;

void timer0_ISR (void) __irq;    // Generate Interrupt 
void timer1_ISR (void) __irq;    // Generate Interrupt 

void temporizador_iniciar(void){
		timer0_int_count = 0;
		// configuration of Timer 0
		T0MR0 = 1499;                        		// Interrumpe cada 1 ms = 1.500-1 counts
    T0MCR = 3;                     							// Generates an interrupt and resets the count when the value of MR0 is reached

    // configuration of the IRQ slot number 0 of the VIC for Timer 0 Interrupt
		VICVectAddr0 = (unsigned long)timer0_ISR;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 4 is the number of the interrupt assigned. Number 4 is the Timer 0 (see table 40 of the LPC2105 user manual  
		VICVectCntl0 = 0x20 | 4;                   
    VICIntEnable = VICIntEnable | 0x00000010;                  // Enable Timer0 Interrupt
		VICIntSelect = VICIntSelect | 0x00000010;										//Enable Fiq for Timer0
	
		timer1_int_count = 0;	
	// configuration of Timer 0
		T1MR0 = 14999;                        		// Interrumpe cada 10 ms = 15000-1 counts
    T1MCR = 3;                     // Generates an interrupt and resets the count when the value of MR0 is reached

    // configuration of the IRQ slot number 0 of the VIC for Timer 0 Interrupt
		VICVectAddr1 = (unsigned long)timer1_ISR;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 4 is the number of the interrupt assigned. Number 4 is the Timer 0 (see table 40 of the LPC2105 user manual  
		VICVectCntl1 = 0x20 | 5;                   
    VICIntEnable = VICIntEnable | 0x00000020;                  // Enable Timer1 Interrupt
}
//Función que habilita las interrupciones de timer1
void temporizador_empezar(void){
	T1TCR = 1;                             // Timer1 Enable
	timer1_int_count = 0;
	TC_init = T1TC;
}
//Funcion que devuelve el tiempo transcurrido desde el inicio en timer1
int temporizador_leer(void){
	return timer1_int_count*1000+((T1TC-TC_init)/15); //15 counts es un micro
}
//Funcion que para los dos timers
int temporizador_parar(void){
	T1TCR = 0;
	T0TCR = 0;
	return T1TC - TC_init;
}
//RSI del timer1
void timer1_ISR (void) __irq {
    timer1_int_count++;											//Suma 1 al contador de ticks de timer1
    T1IR = 1;                              // Clear interrupt flag
    VICVectAddr = 0;                            // Acknowledge Interrupt
}
//Funcion que establece el primer periodo de timer0
void temporizador_periodo(int periodo){
		T0TCR = 1;                             			// Timer0 Enable
	  periodo_alarm = periodo;
		siguiente_periodo = timer0_int_count + periodo_alarm;
}

/* Timer Counter 0 Interrupt executes each 10ms @ 60 MHz CPU Clock */
//RSI del timer 0, la cual viene por FIQ
void timer0_ISR (void) __irq {
    timer0_int_count++;					//Suma 1 al contador de ticks de timer0
		if (timer0_int_count != 0 && timer0_int_count == siguiente_periodo){	// Cuando llega al final controlamos las alarmas
				struct EventInfo timer_event;
				timer_event.idEvento = ID_timer_0;
				cola_guardar_eventos(timer_event.idEvento,timer_event.auxData);//Encolamos un evento que resta 1 a cada periodo de las alarmas activas
				siguiente_periodo = timer0_int_count + periodo_alarm;//Actualizamos la próxima vez que va a entrar al if para restar a las alamas
		}
    T0IR = 1;                              			// Clear interrupt flag
    VICVectAddr = 0;                            // Acknowledge Interrupt
}
//Funcion gettime por SWI
uint32_t __swi(0) clock_gettime(void);

uint32_t __SWI_0 (void) {
	return temporizador_leer();
}
