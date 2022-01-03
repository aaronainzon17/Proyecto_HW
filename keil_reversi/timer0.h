void temporizador_iniciar(void);
void temporizador_empezar(void);
int temporizador_leer(void);
int temporizador_parar(void);
void temporizador_periodo(int periodo);

//Funcion gettime por SWI
int __swi(0x0) clock_gettime(void);
