#ifndef EVENTOS_H
#define EVENTOS_H
#include <inttypes.h>

struct EventInfo {	
	uint8_t idEvento;
  uint32_t auxData;
	uint32_t timeStamp;
};

enum {
	ID_Alarma,	//0
	ID_EINT1,	//1
	ID_EINT2,	//2
	ID_Alarma_visualizacion, //3
	ID_mostrar_vis,	//4
	ID_bit_val,	//5
	ID_fin_val,	//6
	ID_power_down,	//7
	ID_timer_0, //8
	ID_iddle	//9
};

#endif // EVENTOS_H
