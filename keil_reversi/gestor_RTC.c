#include <LPC210x.H>                      
#include <inttypes.h>
#include "gestor_RTC.h"

void RTC_init(void){
	PREINT = 90; //PREINT = int (PCLK / 32768) - 1; PCLK (3*10^6)
	PREFRAC = 18112;// PREFRAC = PCLK - ([PREINT + 1] � 32768)
	CCR=0x02; // CLKEN = 0,  CTCRST = 1
	CCR=0x01; // CLKEN = 1,  CTCRST = 0
}

int RTC_leer_minutos(void){		//CTIME0 [13:8]
	return (CTIME0 >> 8) & 0x0000003F;	
}

int RTC_leer_segundos(void){		//CTIME0 [0:5]
	return CTIME0 & 0x0000003F;
}
