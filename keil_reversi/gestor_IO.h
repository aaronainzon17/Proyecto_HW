#include "eventos.h"
void gestor_IO_init(void);
void gestor_IO_nueva_jugada(void);
void gestor_IO_jugada_de_borrar(void);
void gestor_IO_visualizacion(void);
void gestor_IO_mostrar_visualizacion(struct EventInfo Evento);
void iniciar(void);
void gestor_IO_validacion_1s(void);
void gestor_IO_escribir_bit_validar(void);
void gestor_IO_escribir_bit_powerdown(void);
void gestor_IO_apagar_bit_powerdown(void);
void gestor_IO_validacion_1s_2(void);
int gestor_IO_leer_de_gpio(int inicio, int fin);
