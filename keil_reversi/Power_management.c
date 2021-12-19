#include <LPC210x.H>  
/* LPC210x definitions */
// Set the processor into power down state 
// The watchdog cannot wake up the processor from power down

extern void Switch_to_PLL(void);
//Función que pone al procesador en PowerDown
void PM_power_down (void)  {
  EXTWAKE = 6; // EXTINT1 and EXTINT2 will awake the processor
	PCON |= 0x02; 
	Switch_to_PLL();
}

//Función que pone al procesador en modo iddle de ahorro energético
void PM_idle (void)  {
	PCON |= 0x01; 
}
