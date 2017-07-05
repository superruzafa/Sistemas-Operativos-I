/*ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
/*บ Planificador de procesos                                                 บ*/
/*บ                                                                          บ*/
/*บ Sistemas operativos I                                                    บ*/
/*บ                                                                          บ*/
/*บ Alfonso Ruzafa Molina                                                    บ*/
/*บ Alejandro Sola V zquez                                                   บ*/
/*บ 3ง A ITI Sistemas                                                        บ*/
/*บ                                                                          บ*/
/*บ Fichero: Colas.h                                                         บ*/
/*บ          Funciones de creaciขn y manipulaciขn de colas y PCBs            บ*/
/*บ                                                                          บ*/
/*ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
#if !defined COLAS_DEF
#define COLAS_DEF

#include <alloc.h>


/*----------------------------------------------------------------------------
Estructuras de las colas y los PCBs */

/* Tipos de prioridad disponibles */
enum Prioridad
{
	TR=0, SYS=1, INT=5, BAT=10
};

/* Estructura de un PCB */
struct PCB
{
	unsigned int numProceso;
	unsigned int totalCPU;
	unsigned int horaLLegada;
	unsigned int usoContinuo;
	unsigned int totalMem;
	unsigned int prio;
	unsigned int tiempoEspera;
	unsigned int hechoCPU;
	unsigned int hechoES;
	struct PCB *sig, *ant;
};
#define INTERACTIVO(x) (SYS< x->prio && x->prio < BAT)

/* Estructura de una cola */
struct Cola
{
	struct PCB *inicio, *fin;
};


/*----------------------------------------------------------------------------
Prototipos de funciones */

/* Funciones de manejo de colas */
struct PCB  *CrearPCB       (int, int, int, int, int, enum Prioridad);
int          ColaVacia      (struct Cola *);
struct Cola *CrearCola      ();
struct PCB  *CabezaCola     (struct Cola *);
void         MeteCola       (struct Cola *, struct PCB *, int (*)());
void         Intercambiar   (struct Cola *, struct PCB *, struct PCB *);
void         IncTiempoEspera(struct Cola *);
struct PCB  *SacaCola       (struct Cola *);
struct PCB  *SacarPCB       (struct Cola *, struct PCB *);
void         DestruirCola   (struct Cola *);

/* Funciones de inserciขn en colas */
int OrdenPrioridad   (struct PCB *, struct PCB *);
int OrdenHLLeg       (struct PCB *, struct PCB *);
int OrdenHLLegP      (struct PCB *, struct PCB *);
int OrdenFIFO        (struct PCB *, struct PCB *);
int OrdenFIFOP       (struct PCB *, struct PCB *);
int OrdenSJF         (struct PCB *, struct PCB *);
int OrdenSJFP        (struct PCB *, struct PCB *);
int OrdenSRT         (struct PCB *, struct PCB *);
int OrdenSRTP        (struct PCB *, struct PCB *);


/*----------------------------------------------------------------------------
Funciones de manejo de colas */

/* Devuelve un PCB con relleno con los datos que se le pasan */
struct PCB *CrearPCB(int nProc, int hLLega, int tMem, int tCPU, int usoCont, enum Prioridad prio)
{
	static struct PCB *nuevoPCB;

	nuevoPCB=(struct PCB *)malloc(sizeof(struct PCB));

	nuevoPCB->numProceso=nProc;
	nuevoPCB->totalCPU=tCPU;
	nuevoPCB->horaLLegada=hLLega;
	nuevoPCB->usoContinuo=usoCont;
	nuevoPCB->totalMem=tMem;
	nuevoPCB->prio=prio;

	nuevoPCB->tiempoEspera=0;
	nuevoPCB->hechoCPU=0;
	nuevoPCB->hechoES=0;

	nuevoPCB->sig=NULL;
	nuevoPCB->ant=NULL;

	return(nuevoPCB);
}

/* Devuelve si la cola est  vacกa */
int ColaVacia(struct Cola *cola)
{
	return (cola->inicio==NULL);
}

/* Devuelve una cola vacกa */
struct Cola *CrearCola()
{
	static struct Cola *nuevaCola;

	nuevaCola=(struct Cola *)malloc(sizeof(struct Cola));

	nuevaCola->inicio=NULL;
	nuevaCola->fin=NULL;

	return (nuevaCola);
}

/* Devuelve el primer PCB de una cola */
struct PCB *CabezaCola(struct Cola *cola)
{
	return (cola->inicio);
}

/* Mete un PCB en una cola orden ndolo en ella segฃn la funciขn <orden> */
void MeteCola(struct Cola *cola, struct PCB *proceso, int (*orden)(struct PCB *, struct PCB *))
{
	if (cola->inicio==NULL)
	{
		cola->inicio=proceso;
		proceso->ant=NULL;
	}
	else
	{
		cola->fin->sig=proceso;
		proceso->ant=cola->fin;
	}

	cola->fin=proceso;
	proceso->sig=NULL;

	while (proceso->ant!=NULL && orden (proceso->ant, proceso))
		Intercambiar(cola, proceso->ant, proceso);
}

/* Intercambia dos PCBs dentro de una misma cola */
void Intercambiar(struct Cola *cola, struct PCB *p1, struct PCB *p2)
{
	if (cola->inicio==p1)
		cola->inicio=p2;

	if (cola->fin==p2)
		cola->fin=p1;

	p1->sig=p2->sig;
	p2->ant=p1->ant;

	if (p1->sig!=NULL)
		p1->sig->ant=p1;

	if (p2->ant!=NULL)
		p2->ant->sig=p2;

	p2->sig=p1;
	p1->ant=p2;
}

/* Devuelve el primer PCB sac ndolo de una cola */
struct PCB *SacaCola(struct Cola *cola)
{
	static struct PCB *proceso;

	proceso=cola->inicio;

	if (cola->inicio!=NULL)
	{
		cola->inicio=cola->inicio->sig;

		proceso->sig=NULL;
		proceso->ant=NULL;

		if (cola->inicio==NULL)
			cola->fin=NULL;
		else
			cola->inicio->ant=NULL;

	}
	return (proceso);
}

/* Devuelve un PCB que saca de una cola */
struct PCB *SacaPCB(struct Cola *cola, struct PCB *pcb)
{
	if (cola->inicio==pcb)
	{
		cola->inicio=cola->inicio->sig;
		if (cola->inicio==NULL)
			cola->fin=NULL;
		else
			cola->inicio->ant=NULL;
	}
	else
		if (cola->fin==pcb)
		{
			cola->fin=cola->fin->ant;
			if (cola->fin==NULL)
				cola->inicio=NULL;
			else
				cola->fin->sig=NULL;
		}
		else
		{
			pcb->ant->sig=pcb->sig;
			pcb->sig->ant=pcb->ant;
		}

	return (pcb);
}

/* Incrementa el campo >tiempoEspera> de todos los PCBs de una cola */
void IncTiempoEspera(struct Cola *cola)
{
	struct PCB *proceso=cola->inicio;

	while (proceso!=NULL)
	{
		proceso->tiempoEspera++;
		proceso=proceso->sig;
	}
}

/* Libera la memoria ocupada por una cola */
void DestruirCola(struct Cola *cola)
{
	struct PCB *proceso;

	while (cola->inicio!=NULL)
	{
		proceso=cola->inicio;
		free(proceso);
		cola->inicio=cola->inicio->sig;
	}

	free(cola);
}


/*----------------------------------------------------------------------------
Funciones de ordenaciขn en colas */

/*	A las siguientes funciones se le pasan 2 PCBs consecutivos en una cola:

	INICIO -> ... -> pcb1 -> pcb2 ... -> FIN

	Las funciones devuelven 1 si pcb2 debe estar antes que pc1, por prioridad,
SJF... */

/* Devuelve 1 si pcb2 tiene m s prioridad que pcb1 */
int OrdenPrioridad(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->prio > pcb2->prio);
}

/* Devuelve 1 si pcb1 llega despus que pcb2 */
int OrdenHLLeg(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->horaLLegada > pcb2->horaLLegada);
}

/* Devuelve 1 si pcb2 es igual de prioritario que pcb1 y llega antes o si
pcb2 es m s prioritario que pcb1 */
int OrdenHLLegP(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->prio==pcb2->prio && OrdenHLLeg(pcb1, pcb2) ||
		OrdenPrioridad(pcb1, pcb2));
}

/* Siempre devuelve 0 */
int OrdenFIFO()
{
	return (0);
}

/* Devuelve 1 si pcb2 es m s prioritario que pcb1 */
int OrdenFIFOP(struct PCB *pcb1, struct PCB *pcb2)
{
	return (OrdenPrioridad(pcb1, pcb2));
}

/* Devuelve 1 si pcb2 tiene menos cขmputo que pcb1 */
int OrdenSJF(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->totalCPU > pcb2->totalCPU);
}

/* Devuelve 1 si a pcb2 le queda menos para acabar que a pcb2 */
int OrdenSRT(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->totalCPU-pcb1->hechoCPU > pcb2->totalCPU-pcb2->hechoCPU);
}

/* Devuelve 1 si pcb2 tiene la misma prioridad que pcb1 y tiene menos cขmputo o
si pcb2 tiene m s prioridad que pcb1*/
int OrdenSJFP(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->prio==pcb2->prio && OrdenSJF(pcb1, pcb2) ||
			OrdenPrioridad(pcb1, pcb2));
}

/* Devuelve 1 si pcb2 es igual de prioritario que pcb1 y le queda menos para acabar
o si pcb2 es m s prioritario */
int OrdenSRTP(struct PCB *pcb1, struct PCB *pcb2)
{
	return (pcb1->prio ==pcb2->prio && OrdenSRT(pcb1, pcb2) ||
			OrdenPrioridad(pcb1, pcb2));
}


#endif