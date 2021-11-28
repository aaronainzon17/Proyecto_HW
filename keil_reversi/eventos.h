#ifndef EVENTOS_H
#define EVENTOS_H
#include <inttypes.h>

struct EventInfo {	
	uint8_t idEvento;
  uint32_t auxData;
	uint32_t timeStamp;
};

enum {
	ID_Alarma,
	ID_EINT1,
	ID_EINT2,
	Alarma_visualizacion,
	mostrar_vis,
	bit_val,
	fin_val,
	power_down
};

#endif // EVENTOS_H
