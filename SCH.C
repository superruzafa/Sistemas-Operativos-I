/*ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
/*º Planificador de procesos                                                 º*/
/*º                                                                          º*/
/*º Sistemas operativos I                                                    º*/
/*º                                                                          º*/
/*º Alfonso Ruzafa Molina                                                    º*/
/*º Alejandro Sola V zquez                                                   º*/
/*º 3§ A ITI Sistemas                                                        º*/
/*º                                                                          º*/
/*º Fichero: Sch.c                                                           º*/
/*º          Programa principal                                              º*/
/*º                                                                          º*/
/*ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <math.h>

#include "colas.h"
#include "memoria.h"
#include "util.h"
#include "interfaz.h"

#define TIEMPORETRASO 0

enum Estado
{
	CPU, DISP, LIST, BLOQ, SWAP
};

/* Configuraci¢n de la simulacion */
char                 entrada[30]="carga98.sch";
char                 salida[30]="sucesos.txt";
enum ModoSimulacion  simulacion=MONOPRO_SINES;
enum Politica        politica=FCFS;
enum GestorMemoria   gestMem=PARTICION_DINAMICA;
unsigned int         tio=5;
unsigned int         memoria=512;
unsigned int         quantum=5;
enum AjusteParticion ajuste=PRIMER_AJUSTE;
unsigned int         pcbs=20;

/* Colas del sistema */
struct Cola *carga,
			*listos,
			*bloqueados,
			*swap,
			*finalizados;

/* Un procesador y un recurso */
struct PCB 	*procesador,
			*dispositivo;

/* La memoria del sistema */
void *RAM;

/* Variables internas */
unsigned int  contQuantum;
unsigned long totalComputo, hechoComputo=0;
unsigned int  procEjec=0;
unsigned int  procesosSimulados=0;
unsigned int  cambiosCTX=0;
unsigned long cpuUtil=0;
unsigned long reloj=0;

/* Funciones para insertar en las colas y para hacer la estad¡stica */
int (*ordenCola)();
int (*ordenComp)();


/*----------------------------------------------------------------------------
Prototipos de funciones */

void main(int, char **);
void DibujarEstado(enum Estado);
void LeerTecla();
void SacarParametros(int, char **);
void DibujarPantalla();
int  EscribirSucesos();
void MostrarAyuda();
void SeleccionarOrdenColas();


/*----------------------------------------------------------------------------
Cuerpo de funciones */

void main(int nArg, char *arg[])
{
	SacarParametros(nArg, arg);
	if (EntrarDatos(entrada, salida, &simulacion, &politica, &quantum, &gestMem, &ajuste, &memoria, &tio, &pcbs)==ESCAPE)
	{
		clrscr();
		exit(0);
	}

	SeleccionarOrdenColas();

	/* Creamos los estados */
	listos=CrearCola();
	bloqueados=CrearCola();
	swap=CrearCola();
	finalizados=CrearCola();
	procesador=NULL;
	dispositivo=NULL;
	RAM=CrearMemoria(gestMem, memoria, ajuste);

	carga=AbrirSimulacion(entrada);
	totalComputo=TotalComputo(carga);

	DibujarPantalla();
	DibujarMemoria(MapaMemoria(RAM, gestMem));
	DibujarProgreso(0);
	EscribirReloj(reloj);
	EscribirPCBs(pcbs);
	EscribirCCTX(cambiosCTX);
	EscribirProcEjec(procEjec);

	if (!ColaVacia(carga))
		EscribirEvento(NULL, "Simulaci¢n cargada");
	else
	{
		EscribirEvento(NULL, "Error al cargar el fichero");
		LeerTecla();
		clrscr();
		exit(0);
	}

	LeerTecla();

/*----------------------------------------------------------------------------
SIMULACION */

	/* Se ejecuta mientras queden procesos en bloqueados, en listos, en el
	procesador, en el dispositivo, en swap o en carga.

	S¢lo se pasa por los estados con el trazo doble en MULTIPROCESO */



/*             ÚÄÄÄÄÄÄÄÄ¿  ÚÄÄÄÄÄÄÄÄÄ¿  ÚÄÄÄÄÄÄÄÄÄÄÄ¿  ÉÍÍÍÍÍÍÍÍ»			  */
/*         ÚÄÄ>³ChequearÃÄ>³Chequear ÃÄ>³IntercambioÃÄ>ºApropiarÌÄÄÄ¿         */
/*         ³   ³  CPU   ³  ³Disposit.³  ³de procesos³  º  CPU   º   ³         */
/*         ³   ÀÄÄÄÄÄÄÄÄÙ  ÀÄÄÄÄÄÄÄÄÄÙ  ÀÄÄÄÄÄÄÄÄÄÄÄÙ  ÈÍÍÍÍÍÍÍÍ¼   ³         */
/*         ³   ÚÄÄÄÄÄÄÄÄ¿  ÉÍÍÍÍÍÍÍÍÍ»  ÉÍÍÍÍÍÍÍÍÍÍÍ»  ÉÍÍÍÍÍÍÍÍÍ»  ³         */
/*         ÀÄÄÄ´Tiempo++³<Ä¹Despacharº<Ä¹Admitir m sº<Ä¹Despacharº<ÄÙ         */
/*             ³        ³  º  listos º  º procesos  º  ºbloquead.º            */
/*             ÀÄÄÄÄÄÄÄÄÙ  ÈÍÍÍÍÍÍÍÍÍ¼  ÈÍÍÍÍÍÍÍÍÍÍÍ¼  ÈÍÍÍÍÍÍÍÍÍ¼            */


	while (!ColaVacia(carga) ||
		!ColaVacia(listos) ||
		!ColaVacia(bloqueados) ||
		!ColaVacia(swap) ||
		procesador!=NULL ||
		dispositivo!=NULL)
	{

/*----------------------------------------------------------------------------
ATENDER A LA CPU */


		/* Si hay alg£n proceso en la CPU */
		if (procesador!=NULL)

			/* Si el proceso ha acabado su c¢mputo */
			if (procesador->hechoCPU==procesador->totalCPU)
			{
				EscribirEvento(procesador, "_ ha acabado su c¢mputo");

				LiberarMemoria(RAM, procesador, gestMem);
				DibujarMemoria(MapaMemoria(RAM, gestMem));

				MeteCola(finalizados, procesador, ordenComp);
				procesador=NULL;

				EscribirPCBs(++pcbs);

				EscribirProcEjec(--procEjec);
				DibujarEstado(CPU);

				LeerTecla();

			}
			else
				if (simulacion!=MONOPRO_SINES &&
					procesador->hechoCPU!=0 &&
					procesador->hechoCPU%procesador->usoContinuo==0)
				{
					if (simulacion==MONOPRO_CONES)
					{
						EscribirEvento(procesador, "_ accede al recurso");
						procesador->hechoES=0;

						dispositivo=procesador;
						DibujarEstado(DISP);
					}
					else
					{
						/* Se le suma prioridad */
						if (INTERACTIVO(procesador) &&
							procesador->prio>SYS+1)
							procesador->prio--;

						EscribirEvento(procesador, "Bloqueo de _");

						MeteCola(bloqueados, procesador, ordenCola);
						DibujarEstado(BLOQ);
					}
					procesador=NULL;

					EscribirCCTX(++cambiosCTX);
					DibujarEstado(CPU);

					LeerTecla();
				}


/*----------------------------------------------------------------------------
ATENDER AL DISPOSITIVO */

		if (dispositivo!=NULL &&
			dispositivo->hechoES%tio==0 &&
			dispositivo->hechoES!=0)
		{
			EscribirEvento(dispositivo, "Despertar de _");

			switch(simulacion)
			{
				case MONOPRO_CONES:
					procesador=dispositivo;
					DibujarEstado(CPU);

					break;

				case MULTIPRO_SINPRIO:
				case MULTIPRO_CONPRIO:

					/* Se le suma prioridad */
					if (INTERACTIVO(dispositivo) &&
						dispositivo->prio>SYS+1)
						dispositivo->prio--;

					MeteCola(listos, dispositivo, ordenCola);
					DibujarEstado(LIST);

					break;
			}

			dispositivo=NULL;

			EscribirCCTX(++cambiosCTX);
			DibujarEstado(DISP);

			LeerTecla();
		}



/*----------------------------------------------------------------------------
SACAR LOS PROCESOS QUE SE ENCUENTREN INTERCAMBIADOS */

		switch(simulacion)
		{
			case MONOPRO_SINES:
			case MONOPRO_CONES:

				if (!ColaVacia(swap) &&
					procEjec==0)
				{
					EscribirEvento(swap->inicio, "_ ha sido intercambiado");

					AsignarMemoria(RAM, CabezaCola(swap), gestMem);
					DibujarMemoria(MapaMemoria(RAM, gestMem));

					procesador=SacaCola(swap);

					EscribirCCTX(++cambiosCTX);
					DibujarEstado(CPU);
					DibujarEstado(SWAP);

					EscribirProcEjec(++procEjec);

					LeerTecla();
				}

				break;

			case MULTIPRO_SINPRIO:
			case MULTIPRO_CONPRIO:

				while (!ColaVacia(swap) &&
					CabezaCola(swap)->totalMem <= MemoriaDisponible(RAM, gestMem))
				{
					EscribirEvento(swap->inicio, "_ ha sido intercambiado");

					AsignarMemoria(RAM, CabezaCola(swap), gestMem);
					DibujarMemoria(MapaMemoria(RAM, gestMem));

					MeteCola(listos, SacaCola(swap), ordenCola);
					EscribirProcEjec(++procEjec);

					EscribirCCTX(++cambiosCTX);
					DibujarEstado(LIST);
					DibujarEstado(SWAP);

					LeerTecla();
				}

				break;
		}


/*----------------------------------------------------------------------------
APROPIAR LA CPU */

		if (MULTIPROCESO(simulacion) &&
			procesador!=NULL)

			if (politica==SRT && !ColaVacia(listos) &&
			(simulacion==MULTIPRO_SINPRIO && OrdenSRT(procesador, CabezaCola(listos)) ||
			simulacion==MULTIPRO_CONPRIO &&	OrdenSRTP(procesador, CabezaCola(listos)) ))
			{

				/* Se le incrementa prioridad */
				if (INTERACTIVO(procesador) &&
					procesador->prio>SYS+1)
					procesador->prio--;

				EscribirEvento(listos->inicio, "_ se apropia de la CPU");

				MeteCola(listos, procesador, ordenCola);
				procesador=NULL;

				EscribirCCTX(++cambiosCTX);
				DibujarEstado(LIST);
				DibujarEstado(CPU);

				LeerTecla();
			}
			else
				if (politica==RR &&
					contQuantum==quantum)
				{
					/* Se le resta prioridad */
					if (INTERACTIVO(procesador) &&
						procesador->prio<BAT-1)
						procesador->prio++;

					EscribirEvento(procesador, "Tiempo excedido de _");

					MeteCola(listos, procesador, ordenCola);
					procesador=NULL;

					EscribirCCTX(++cambiosCTX);
					DibujarEstado(LIST);
					DibujarEstado(CPU);

					LeerTecla();
				}



/*----------------------------------------------------------------------------
ATENDER A LA COLA DE BLOQUEADOS */

		if (!ColaVacia(bloqueados) &&
			dispositivo==NULL)
		{
			EscribirEvento(bloqueados->inicio, "_ ha conseguido el acceso al recurso");

			dispositivo=SacaCola(bloqueados);

			EscribirCCTX(++cambiosCTX);
			DibujarEstado(DISP);
			DibujarEstado(BLOQ);

			LeerTecla();
		}


/*----------------------------------------------------------------------------
ADMITIR NUEVOS PROCESOS */

/*		El bucle se ejecuta hasta que todos los procesos que llegan en el
	instante <reloj> han sido metidos en el sistema.

		A continuaci¢n se muestra el camino que siguen los procesos hasta que
	son	atendidos: */


/* ÉÍÍÍÍÍÍ»                                                                 */
/* ºINICIOº        ÚÄÄÄÄÄÄÄÄ¿                                               */
/* ÈÍÍËÍÍÍ¼    no  ³Eliminar³                                               */
/*    ³        ÚÄÄ>³proceso ³                                               */
/*    v        ³   ÀÄÄÄÄÄÄÄÄÙ                                               */
/*ÚÄÄÄÄÄÄÄÄÄÄ¿ ³              monoÚÄÄÄÄÄÄÄÄ¿	           1                */
/*³ ¨Cabe el ³ ³                  ³¨N§ pro-ÃÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³proceso enÃÄ´                Ú>³cesos en³   ³                   ³        */
/*³ memoria? ³ ³ ÚÄÄÄÄÄÄÄÄÄÄÄÄ¿ ³ ³sistema?³   ³    ÚÄÄÄÄÄÄÄÄ¿     ³        */
/*ÀÄÄÄÄÄÄÄÄÄÄÙ ³ ³ ¨Ambiente  ³ ³ ÀÄÄÄÄÄÄÄÄÙ   ³ 0  ³Meter en³     ³        */
/*             À>³mono o multiÃÄ´              ÀÄÄÄ>³   CPU  ³     v        */
/*             s¡³  proceso?  ³ ³                   ÀÄÄÄÄÄÄÄÄÙ ÚÄÄÄÄÄÄÄÄ¿   */
/*               ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ ³multi                         ³Meter en³   */
/* ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ                   ÚÄÄÄÄÄÄÄÄ¿ ³  SWAP  ³   */
/* ³  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿                              ³Meter en³ ÀÄÄÄÄÄÄÄÄÙ   */
/* ³  ³¨Ambiente mul-³ no                           ³ LISTOS ³     ^        */
/* ÀÄ>³tiproceso con ÃÄÄÄÄÄÄÄ¿                      ÀÄÄÄÄÄÄÄÄÙ   s¡³        */
/*    ³  prioridad?  ³       ³                             ^       ³        */
/*    ÀÄÄÄÄÄÄÂÄÄÄÄÄÄÄÙ   no  v  ÚÄÄÄÄÄÄÄÄÄ¿s¡  ÚÄÄÄÄÄÄÄ¿ s¡³       ³        */
/*           ³ s¡       ÚÄÄÄÄÄÄ>³¨Hay PCBsÃÄÄÄ>³ ¨Hay  ÃÄÄÄÁÄÄÄÄÄÄ>³        */
/*           ³          ³       ³ libres? ³    ³memoria³        no ³        */
/*           v          ³       ÀÄÄÄÄÂÄÄÄÄÙ    ³libre? ³           ³        */
/*    ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿  ³            ³no       ÀÄÄÄÄÄÄÄÙ           ³        */
/*    ³¨Hay  en  SWAP³  ³            v                             ³        */
/*    ³alg£n proceso ÃÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*    ³+ prioritario?³	  s¡                                                */
/*    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ                                                      */

		while (!ColaVacia(carga) &&
			CabezaCola(carga)->horaLLegada==reloj)
		{
			if (CabezaCola(carga)->totalMem <= MemoriaMaxima(RAM, gestMem) &&
				pcbs>0)
			{

				procesosSimulados++;
				EscribirPCBs(--pcbs);
				/* En ambiente MONOPROCESO, s¢lo puede haber 1 proceso
				compitiendo */
				if MONOPROCESO(simulacion)
				{
					/* Si no hay ning£n proceso, se mete en el procesador */
					if (procEjec==0)
					{
						EscribirEvento(carga->inicio, "Ha llegado el proceso _");

						AsignarMemoria(RAM, CabezaCola(carga), gestMem);
						DibujarMemoria(MapaMemoria(RAM, gestMem));

						procesador=SacaCola(carga);
						EscribirProcEjec(++procEjec);
						DibujarEstado(CPU);
					}

					/* En otro caso, se intercambia */
					else
					{
						EscribirEvento(carga->inicio, "_ ha llegado y ha sido intercambiado");

						MeteCola(swap, SacaCola(carga), ordenCola);
						DibujarEstado(SWAP);
					}
				}

				/* Ambiente MULTIPROCESO */
				else
				{
					/* El proceso se mete en la cola de listos si el proceso
					que llega es m s prioritario que el primero de swap, si
					hay PCBs libres y si hay memoria disponible para ‚l. */

					if (carga->inicio->totalMem<=MemoriaDisponible(RAM, gestMem) &&
						(simulacion==MULTIPRO_SINPRIO ||
						simulacion==MULTIPRO_CONPRIO &&
						(!ColaVacia(swap) && ordenCola(carga->inicio, swap->inicio) ||
						ColaVacia(swap) )))
					{
						EscribirEvento(carga->inicio, "Ha llegado el proceso _");

						AsignarMemoria(RAM, CabezaCola(carga), gestMem);
						DibujarMemoria(MapaMemoria(RAM, gestMem));

						MeteCola(listos, SacaCola(carga), ordenCola);

						EscribirProcEjec(++procEjec);
						DibujarEstado(LIST);
					}

					/* Ahora mismo no hay memoria para el proceso */
					else
					{
						EscribirEvento(carga->inicio, "_ ha llegado y ha sido intercambiado");

						MeteCola(swap, SacaCola(carga), ordenCola);
						DibujarEstado(SWAP);
					}
				}
			}

			/* El proceso requiere mas memoria de la que el sistema ofrece */
			else
			{
				if (pcbs==0)
					EscribirEvento(carga->inicio, "Se ha eliminado _ por falta PCBs");
				else
					EscribirEvento(carga->inicio, "_ no cabe en memoria y se ha eliminado");

				totalComputo-=carga->inicio->totalCPU;

				carga->inicio->tiempoEspera=MAXINTEGER;
				MeteCola(finalizados, SacaCola(carga), ordenComp);
			}

			LeerTecla();
		}



/*----------------------------------------------------------------------------
ATENDER A LA COLA DE LISTOS */

		if (MULTIPROCESO(simulacion) &&
			!ColaVacia(listos) &&
			procesador==NULL)
		{
			/* Se le quita prioridad */
			if ((politica==SRT || politica==RR) &&
				INTERACTIVO(listos->inicio) &&
				listos->inicio->prio<BAT-1)
				listos->inicio->prio++;

			EscribirEvento(listos->inicio, "Despacho del proceso _");

			procesador=SacaCola(listos);

			if (politica==RR)
				contQuantum=0;

			EscribirCCTX(++cambiosCTX);
			DibujarEstado(CPU);
			DibujarEstado(LIST);

			LeerTecla();
		}


/*----------------------------------------------------------------------------
INCREMENTAR EL TIEMPO */

		if (procesador!=NULL)
		{
			procesador->hechoCPU++;
			DibujarProgreso(++hechoComputo*100/totalComputo);
			cpuUtil++;
		}

		if (dispositivo!=NULL)
			dispositivo->hechoES++;

		IncTiempoEspera(listos);
		IncTiempoEspera(bloqueados);
		IncTiempoEspera(swap);

		if (MULTIPROCESO(simulacion) &&
			politica==RR &&
			procesador!=NULL)
			contQuantum++;

		reloj++;
		EscribirReloj(reloj);

	}

	EscribirEvento(NULL, "Simulaci¢n Finalizada");

	getch();

	if (EscribirSucesos())
		EscribirEvento(NULL, "Archivo de salida creado");
	else
		EscribirEvento(NULL, "Error al escribir el archivo de salida");

	getch();

	DestruirCola(finalizados);

	clrscr();
}


void DibujarEstado(enum Estado e)
{

	switch(e)
	{
		case CPU:
			EscribirVacio(9, 1, 4, 3);
			DibujarPCBs(procesador, 9, 1);
			break;

		case DISP:
			EscribirVacio(21, 1, 4, 3);
			DibujarPCBs(dispositivo, 21, 1);
			break;

		case BLOQ:
			EscribirVacio(9, 7, COLUMNAS-1-9, 3);
			DibujarPCBs(bloqueados->inicio, 9, 7);
			break;

		case LIST:
			EscribirVacio(9, 4, COLUMNAS-1-9, 3);
			DibujarPCBs(listos->inicio, 9, 4);
			break;

		case SWAP:
			EscribirVacio(33, 1, COLUMNAS-1-33, 3);
			DibujarPCBs(swap->inicio, 33, 1);
			break;
	}
}


void LeerTecla()
{
	static int returnPulsado=0;

	int tecla;

	if (kbhit() || !returnPulsado)
	{
		tecla=getch();

		returnPulsado=0;

		if (tecla==ESCAPE)
		{
			clrscr();
			exit(0);
		}
		else
			returnPulsado=(tecla==RETORNO);
	}
	else
		delay(TIEMPORETRASO);
}



void SacarParametros(int n, char **arg)
{

	int p=1;

/*

-e Entrada
-l Salida
-s Simulaci¢n
-p Politica
-q Quantum
-g Gestor Memoria
-a Ajuste
-t TIO
-m Memoria
-c PCBs
-? Ayuda

*/

	while (p<n)
	{
		if (arg[p][0]=='-')
			switch(arg[p++][1])
			{
				case '?':
					MostrarAyuda();
					break;

				case 'e':
					strcpy(entrada, arg[p]);
					break;

				case 'l':
					strcpy(salida, arg[p]);
					break;

				case 's':
					simulacion=EnumSimulacion(arg[p]);
					break;

				case 'p':
					politica=EnumPolitica(arg[p]);
					break;

				case 'q':
					quantum=atoi(arg[p]);
					break;

				case 'g':
					gestMem=EnumGestorMemoria(arg[p]);
					break;

				case 'a':
					ajuste=EnumAjusteParticion(arg[p]);
					break;

				case 't':
					tio=atoi(arg[p]);
					break;

				case 'm':
					memoria=atoi(arg[p]);
					break;

				case 'c':
					pcbs=atoi(arg[p]);
					break;
			}
			p++;
	}

	if (MONOPROCESO(simulacion))
	{
		gestMem=GESNINGUNO;
		politica=FCFS;
		ajuste=AJNINGUNO;
		quantum=0;

		if (simulacion==MONOPRO_SINES)
			tio=0;
	}
	else
		if (gestMem==PAGINACION ||
			gestMem==GESNINGUNO)
		{
			gestMem=PAGINACION;
			ajuste=AJNINGUNO;
		}
		else
			if (ajuste==AJNINGUNO)
				ajuste=PRIMER_AJUSTE;

}


void DibujarPantalla()
{
	char nombre [11][30];
	char *param[11];
	int cont;

	strcpy(                      nombre[0], entrada);
	strcpy(                      nombre[1], salida);
	NombreSimulacion(simulacion, nombre[2]);
	NombrePolitica(politica,     nombre[3]);
	NombreGestorMemoria(gestMem, nombre[4]);
	itoa(tio,                    nombre[5], 10);
	itoa(memoria,                nombre[6], 10);
	if (quantum==0)
		strcpy(                  nombre[7], "Desactivado");
	else
		itoa(quantum,            nombre[7], 10);
	NombreAjuste(ajuste,         nombre[8]);
	itoa(pcbs,                   nombre[9], 10);
	dtoa((double)memoria/DIVMEMORIA,     nombre[10], 4);

	for (cont=0; cont<11; cont++)
		param[cont]=nombre[cont];

	DibujarInterfaz(param);
}


int EscribirSucesos()
{
	#define TASACPU(x) (double)x->totalCPU / (x->totalCPU + x->tiempoEspera)*100

	FILE *f;
	char buffer[130];
	struct PCB *proceso=finalizados->inicio;

	int cont, longi;

/*----------------------------------------------------------------------------
DATOS DE LA SIMULACION */

	f=fopen(salida, "wt");

	if (f==NULL)
		return(0);

	fputs("Datos de la simulacion:\n-----------------------\n", f);

	fputs("    Archivo de carga: ", f);
	fputs(entrada, f); fputs("\n", f);

	fputs("    Simulacion: ", f);
	fputs(NombreSimulacion(simulacion, buffer), f); fputs("\n", f);

	if (MULTIPROCESO(simulacion))
	{
		fputs("    Politica: ", f);
		fputs(NombrePolitica(politica, buffer), f); fputs("\n", f);
	}

	if (MULTIPROCESO(simulacion) &&
		politica==RR)
	{
		fputs("    Quantum: ", f);
		fputs(itoa(quantum, buffer, 10), f); fputs("\n", f);
	}

	fputs("    Gestor de memoria: ", f);
	fputs(NombreGestorMemoria(gestMem, buffer), f); fputs("\n", f);

	if (gestMem==PARTICION_ESTATICA ||
		gestMem==PARTICION_DINAMICA)
	{
		fputs("    Ajuste: ", f);
		fputs(NombreAjuste(ajuste, buffer), f); fputs("\n", f);
	}

	fputs("    TIO: ", f);
	fputs(itoa(tio, buffer, 10), f); fputs("\n", f);

	fputs("    Memoria: ", f);
	fputs(itoa(memoria, buffer, 10), f); fputs("\n\n", f);

/*----------------------------------------------------------------------------
EFICIENCIA */

	fputs("Eficiencia:\n----------\n", f);

	fputs("    Tiempo de simulacion: ", f);
	fputs(itoa(reloj, buffer, 10), f); fputs("\n", f);

	fputs("    CPU util: ", f);
	fputs(itoa(cpuUtil, buffer, 10), f); fputs("\n", f);

	fputs("    Cambios de contexto: ", f);
	fputs(itoa(cambiosCTX, buffer, 10), f); fputs("\n", f);

	fputs("    Eficiencia CPU: ", f);
	fputs(itoa((int)((double)cpuUtil/(reloj+cambiosCTX)*100), buffer, 10), f); fputs("%\n\n", f);

/*----------------------------------------------------------------------------
TIEMPOS DE ESPERA EN COLAS */


	fputs("Tiempo de espera en colas:\n--------------------------\n", f);

	fputs("    N§ pro   Tmp. esp\n", f);
	fputs("    ------   --------\n", f );
	while (proceso!=NULL)
	{
		fputs("      ", f);

		fputs(itoa(proceso->numProceso, buffer, 10), f);
		fputs(Espacios(buffer, 10-strlen(itoa(proceso->numProceso, buffer, 10))), f);

		fputs(itoa(proceso->tiempoEspera, buffer, 10), f);
		fputs(Espacios(buffer, 9-strlen(itoa(proceso->tiempoEspera, buffer, 10))), f);

		fputs("\n", f);

		proceso=proceso->sig;
	}

	fputs("\n", f);

/*----------------------------------------------------------------------------
PRODUCTIVIDAD */

	fputs("Productividad:\n--------------\n", f);

	fputs("    N§ de procesos simulados: ", f);
	fputs(itoa(procesosSimulados, buffer, 10), f); fputs("\n", f);

	fputs("    Tiempo simulacion: ", f);
	fputs(itoa(reloj, buffer, 10), f); fputs("\n", f);

	fputs("    Productividad: ", f);
	fputs(dtoa((double)procesosSimulados / reloj*1000, buffer, 3), f); fputs(" x 1000\n\n", f);

/*----------------------------------------------------------------------------
TASA DE CPU */

	fputs("Tasa de CPU:\n-----------\n", f);
	fputs("    N§ proc    TasaCPU\n", f);
	fputs("    -------    -------\n", f);
	proceso=finalizados->inicio;

	while (proceso!=NULL)
	{
		fputs(Espacios(buffer, 6), f);

		fputs(itoa(proceso->numProceso, buffer, 10), f);
		fputs(Espacios(buffer, 10-strlen(itoa(proceso->numProceso, buffer, 10))), f);

		fputs(itoa(TASACPU(proceso), buffer, 10), f); fputs("%\n", f);

		proceso=proceso->sig;
	}

	fclose(f);

	return(1);
}


void MostrarAyuda()
{
	clrscr();

	printf("\
Simulador de Planificador de procesos\n\n\
Opciones:\n\n\
-s <SIMULACION>   [ MONOSINES | MONOCONES | MULTISINP | MULTICONP ]\n\
-p <POLITICA>     [ FCFS | SJF | SRT | RR ]\n\
-g <GESTOR MEM.>  [ NINGUNO | ESTATICA | DINAMICA | PAGINADO ]\n\
-a <AJUSTE PART.> [ PRIMER | MEJOR | PEOR ]\n\
-t <TIO>\n\
-m <MEMORIA>\n\
-c <N§ PCBs>\n\
-q <QUANTUM>\n\
-e <ARCHIVO DE ENTRADA>\n\
-l <ARCHIVO DE SALIDA>\n\
-? <AYUDA>\n\n");
	exit(0);
}


void SeleccionarOrdenColas()
{
/*                     ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ */
/*                     ³              S I M U L A C I O N E S              ³ */
/*ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³POLITICA³    COLA   ³ MONO_SINES ³ MONO_CONES ³ MULTI_SINP ³ MULTI_CONP ³ */
/*ÃÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³COMPLETADOS³ OrdenHLLeg ³ OrdenHLLeg ³ OrdenHLLeg ³ OrdenHLLegP³ */
/*³  FCFS  ÃÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³   OTRA    ³     X      ³     X      ³ OrdenHLLeg ³ OrdenHLLegP³ */
/*ÃÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³COMPLETADOS³     X      ³     X      ³  OrdenSJF  ³ OrdenSJFP  ³ */
/*³  SJF   ÃÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³   OTRA    ³     X      ³     X      ³  OrdenSJF  ³ OrdenSJFP  ³ */
/*ÃÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³COMPLETADOS³     X     	³     X      ³ OrdenHLLeg ³ OrdenHLLegP³ */
/*³  SRT   ÃÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³   OTRA    ³     X      ³     X      ³  OrdenSRT  ³ OrdenSRTP  ³ */
/*ÃÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³COMPLETADOS³     X      ³     X      ³ OrdenHLLeg ³ OrdenHLLegP³ */
/*³   RR   ÃÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´ */
/*³        ³   OTRA    ³     X      ³     X      ³ OrdenFIFO  ³ OrdenFIFOP ³ */
/*ÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÙ */

	if (MONOPROCESO(simulacion))
	{
		ordenCola=OrdenHLLeg;
		ordenComp=OrdenHLLeg;
	}
	else
		if (simulacion==MULTIPRO_SINPRIO)
			switch (politica)
			{
				case FCFS:
					ordenCola=OrdenHLLeg;
					ordenComp=OrdenHLLeg;
					break;

				case SJF:
					ordenCola=OrdenSJF;
					ordenComp=OrdenSJF;
					break;

				case SRT:
					ordenCola=OrdenSRT;
					ordenComp=OrdenHLLeg;
					break;

				case RR:
					ordenCola=OrdenFIFO;
					ordenComp=OrdenHLLeg;
					break;
			}
		else
			switch (politica)
			{
				case FCFS:
					ordenCola=OrdenHLLegP;
					ordenComp=OrdenHLLegP;
					break;

				case SJF:
					ordenCola=OrdenSJFP;
					ordenComp=OrdenSJFP;
					break;

				case SRT:
					ordenCola=OrdenSRTP;
					ordenComp=OrdenHLLegP;
					break;

				  case RR:
					ordenCola=OrdenFIFOP;
					ordenComp=OrdenHLLegP;
					break;
			}
}





