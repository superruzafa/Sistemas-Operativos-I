/*ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
/*บ Planificador de procesos                                                 บ*/
/*บ                                                                          บ*/
/*บ Sistemas operativos I                                                    บ*/
/*บ                                                                          บ*/
/*บ Alfonso Ruzafa Molina                                                    บ*/
/*บ Alejandro Sola V zquez                                                   บ*/
/*บ 3ง A ITI Sistemas                                                        บ*/
/*บ                                                                          บ*/
/*บ Fichero: Memoria.h                                                       บ*/
/*บ          Funciones para la gestiขn de memoria                            บ*/
/*บ                                                                          บ*/
/*ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
#if !defined(MEMORIA_DEF)
#define MEMORIA_DEF

#include "colas.h"
#include "util.h"
#include <string.h>
#include <math.h>

#define NUM_PART_EST 8
#define MEMORIAPARTICIONADA(x) (x==PARTICION_ESTATICA || x==PARTICION_DINAMICA)
#define DIVMEMORIA 128

/* Enumerado con los tipos de gestiขn de memoria que se simulan */
enum GestorMemoria
{
	GESNINGUNO, PARTICION_ESTATICA, PARTICION_DINAMICA, PAGINACION
};

/* Estado del bloque de memoria */
enum EstadoMemoria
{
	LIBRE, ASIGNADA
};


/* Estructura para el gestor de memoria NINGUNO */
struct MemNinguno
{
	unsigned int memTotal;
	unsigned int memLibre;
};

/* Macro que devuelve una nueva particiขn */
#define NUEVAPARTICION (struct ListaParticiones *)malloc(sizeof(struct ListaParticiones))

/* Estructura que representa una particiขn */
struct ListaParticiones
{
	int id;
	enum EstadoMemoria est;
	int longitud;
	struct ListaParticiones *sig;
};

/* Estructura para el gestor de memoria Particionado */
struct MemParticiones
{
	struct ListaParticiones *mapa;
	enum Ajuste ajuste;
	unsigned int memTotal;
};

/* Enumerado con los tipos de ajuste de particiones */
enum AjusteParticion
{
	AJNINGUNO, PRIMER_AJUSTE, MEJOR_AJUSTE, PEOR_AJUSTE
};

/* Estructura que representa una p gina */
struct ListaPaginas
{
	enum EstadoMemoria est;
	int id;
};

/* Estructura para el gestor de memoria PAGINADO */
struct MemPaginas
{
	struct ListaPaginas mapa[DIVMEMORIA];
	int pagLibres;
	double tamPagina;
	unsigned int memTotal;
};

/*---------------------------------------------------------------------------
Prototipos de funciones */

void *CrearMemoria(enum GestorMemoria, int, enum AjusteParticion);
void AsignarMemoria(void *, struct PCB *, enum GestorMemoria);
void LiberarMemoria(void *, struct PCB *, enum GestorMemoria);
int MemoriaMaxima(void *, enum GestorMemoria);
int MemoriaDisponible(void *, enum GestorMemoria);
enum EstadoMemoria *MapaMemoria(void *, enum GestorMemoria);

char *NombreGestorMemoria (enum GestorMemoria, char *);
enum GestorMemoria EnumGestorMemoria(char *);
char *NombreAjuste(enum AjusteParticion, char *);
enum AjusteParticion EnumAjusteParticion(char *);


/*----------------------------------------------------------------------------
Cuerpo de funciones */


/* Devuelve un gestor de memoria */
void *CrearMemoria(enum GestorMemoria gestMem, int tam, enum AjusteParticion ajuste)
{
	static void *mem=NULL;

	int nuevoTam;
	struct ListaParticiones *part;

	int cont;

	switch (gestMem)
	{
		case GESNINGUNO:
			mem=(struct MemNinguno *)malloc(sizeof(struct MemNinguno));
			((struct MemNinguno *)mem)->memTotal=tam;
			((struct MemNinguno *)mem)->memLibre=tam;

			break;

		case PARTICION_ESTATICA:
			mem=(struct MemParticiones *)malloc(sizeof(struct MemParticiones));

			((struct MemParticiones *)mem)->memTotal=tam;
			((struct MemParticiones *)mem)->ajuste=ajuste;
			((struct MemParticiones *)mem)->mapa=NULL;

			nuevoTam=tam;

			while (nuevoTam>tam/pow(2, NUM_PART_EST-1))
			{
				nuevoTam/=2;

				part=NUEVAPARTICION;
				part->est=LIBRE;
				part->longitud=nuevoTam;
				part->sig=((struct MemParticiones *)mem)->mapa;
				((struct MemParticiones *)mem)->mapa=part;
			}
			/*Las 2 ฃltimas particiones tienen el mismo tamaคo*/
			part=NUEVAPARTICION;
			part->est=LIBRE;
			part->longitud=nuevoTam;
			part->sig=((struct MemParticiones *)mem)->mapa;
			((struct MemParticiones *)mem)->mapa=part;

			break;

		case PARTICION_DINAMICA:

			mem=(struct MemParticiones *)malloc(sizeof(struct MemParticiones));
			((struct MemParticiones *)mem)->memTotal=tam;
			((struct MemParticiones *)mem)->ajuste=ajuste;

			((struct MemParticiones *)mem)->mapa=NUEVAPARTICION;
			((struct MemParticiones *)mem)->mapa->est=LIBRE;
			((struct MemParticiones *)mem)->mapa->longitud=tam;
			((struct MemParticiones *)mem)->mapa->sig=NULL;

			break;

		case PAGINACION:

			mem=(struct MemPaginas *)malloc(sizeof(struct MemPaginas));

			for (cont=0; cont<DIVMEMORIA; cont++)
				((struct MemPaginas *)mem)->mapa[cont].est=LIBRE;

			((struct MemPaginas *)mem)->tamPagina=(double)tam/DIVMEMORIA;
			((struct MemPaginas *)mem)->pagLibres=DIVMEMORIA;
			((struct MemPaginas *)mem)->memTotal=tam;

			break;
	}

	return (mem);
}


/* Asigna memoria en un gestor */
void AsignarMemoria(void *mem, struct PCB *proceso, enum GestorMemoria gestMem)
{
	struct ListaParticiones *busqSeg=NULL, *busqSeg2, *fragExt=NULL;
	int tamFrag;

	int pagNeces;
	int cont;

	/* Para buscar el segmento segฃn criterio */
	if (MEMORIAPARTICIONADA(gestMem))

		switch (((struct MemParticiones *)mem)->ajuste)
		{
			case PRIMER_AJUSTE:

				busqSeg=((struct MemParticiones *)mem)->mapa;

				while (busqSeg!=NULL &&
					!(busqSeg->longitud>=proceso->totalMem &&
					busqSeg->est==LIBRE))
					busqSeg=busqSeg->sig;

				break;

			case MEJOR_AJUSTE:

				busqSeg2=((struct MemParticiones *)mem)->mapa;
				tamFrag=MAXINTEGER;

				while (busqSeg2!=NULL)
				{
					if (busqSeg2->est==LIBRE &&
						busqSeg2->longitud>=proceso->totalMem &&
						busqSeg2->longitud<tamFrag)
					{
						tamFrag=busqSeg2->longitud;
						busqSeg=busqSeg2;
					}
					busqSeg2=busqSeg2->sig;
				}
				break;

			case PEOR_AJUSTE:

				busqSeg2=((struct MemParticiones *)mem)->mapa;
				tamFrag=0;

				while (busqSeg2!=NULL)
				{
					if (busqSeg2->est==LIBRE &&
						busqSeg2->longitud>=proceso->totalMem &&
						busqSeg2->longitud>tamFrag)
					{
						tamFrag=busqSeg2->longitud;
						busqSeg=busqSeg2;
					}
					busqSeg2=busqSeg2->sig;
				}
				break;
		}

	switch (gestMem)
	{
		case GESNINGUNO:
			((struct MemNinguno *)mem)->memLibre-=proceso->totalMem;
			break;

		case PARTICION_ESTATICA:

			busqSeg->id=proceso->numProceso;
			busqSeg->est=ASIGNADA;

			break;

		case PARTICION_DINAMICA:

		/* Si la particiขn en donde se va a ubicar el proceso es m s grande
		que el proceso, entonces se va a producir fragmentaciขn externa */
			if (busqSeg->longitud>proceso->totalMem)
			{
				fragExt=NUEVAPARTICION;

				fragExt->est=LIBRE;
				fragExt->longitud=busqSeg->longitud-proceso->totalMem;
				fragExt->sig=busqSeg->sig;
			}

			busqSeg->est=ASIGNADA;
			busqSeg->id=proceso->numProceso;
			busqSeg->longitud=proceso->totalMem;

			/* Si no se ha producido fragmentaciขn, la siguiente particiขn
			es la misma */
			if (fragExt!=NULL)
				busqSeg->sig=fragExt;

			break;

		case PAGINACION:

			/* Cogemos por exceso las p ginas que necesitemos */
			pagNeces=ceil((double)(proceso->totalMem)/((struct MemPaginas *)mem)->tamPagina);

			/* Gastamos esas p ginas */
			((struct MemPaginas *)mem)->pagLibres-=pagNeces;

			cont=0;
			/* Buscamos secuencialmente p ginas libres para asignar */
			while (pagNeces>0 && cont<DIVMEMORIA)
			{
				if (((struct MemPaginas *)mem)->mapa[cont].est==LIBRE)
				{
					((struct MemPaginas *)mem)->mapa[cont].est=ASIGNADA;
					((struct MemPaginas *)mem)->mapa[cont].id=proceso->numProceso;
					pagNeces--;
				}
				cont++;
			}

			break;
	}
}


/* Libera la memoria ocupada por un proceso */
void LiberarMemoria(void *mem, struct PCB *proceso, enum GestorMemoria gestMem)
{
	struct ListaParticiones *busqSeg, *unirAnt=NULL, *unirSig;

	int cont;

	/* Se encarga de encontrar la particiขn donde est  el proceso */
	if (MEMORIAPARTICIONADA(gestMem))
	{
		busqSeg=((struct MemParticiones *)mem)->mapa;

		while (busqSeg!=NULL &&
			!(busqSeg->id==proceso->numProceso &&
			busqSeg->est==ASIGNADA))
		{
			unirAnt=busqSeg;
			busqSeg=busqSeg->sig;
		}
	}

	switch (gestMem)
	{
		case GESNINGUNO:

			/* Simplemente, incrementamos la memoria libre */
			((struct MemNinguno *)mem)->memLibre+=proceso->totalMem;

			break;

		case PARTICION_ESTATICA:

			/* Liberamos la particiขn en la que se halla */
			busqSeg->est=LIBRE;

			break;

		case PARTICION_DINAMICA:

			/* Liberamos la particiขn en la que se halla */
			busqSeg->est=LIBRE;

			/* Si el siguiente est  libre, los unimos */
			if (busqSeg->sig!=NULL &&
				busqSeg->sig->est==LIBRE)
			{
				unirSig=busqSeg->sig;
				busqSeg->longitud+=unirSig->longitud;
				busqSeg->sig=unirSig->sig;
				free(unirSig);
			}

			/* Si el anterior est  libre, tambin los unimos */
			if (unirAnt!=NULL &&
				unirAnt->est==LIBRE)
			{
				unirAnt->longitud+=busqSeg->longitud;
				unirAnt->sig=busqSeg->sig;
				free(busqSeg);
			}

			break;

		case PAGINACION:

			/* Liberamos las p ginas que utilice el proceso */
			for (cont=0; cont<DIVMEMORIA; cont++)
				if (((struct MemPaginas *)mem)->mapa[cont].id==proceso->numProceso &&
					((struct MemPaginas *)mem)->mapa[cont].est==ASIGNADA)
				{
					((struct MemPaginas *)mem)->mapa[cont].est=LIBRE;
					((struct MemPaginas *)mem)->pagLibres++;
				}
			break;
	}
}



/* Devuelve el nombre de un ajuste */
char *NombreAjuste(enum AjusteParticion ajuste, char *buffer)
{
	switch(ajuste)
	{
		case AJNINGUNO:
			strcpy(buffer, "Ninguno");
			break;

		case PRIMER_AJUSTE:
			strcpy(buffer, "Primer ajuste");
			break;

		case MEJOR_AJUSTE:
			strcpy(buffer, "Mejor ajuste");
			break;

		case PEOR_AJUSTE:
			strcpy(buffer, "Peor ajuste");
			break;

		default:
			strcpy(buffer, "Desconocido");
			break;
	}

	return(buffer);
}


/* Devuelve el enumerado de un ajuste */
enum AjusteParticion EnumAjusteParticion(char *cadena)
{
	static enum AjusteParticion enumP;

	if (strcmp(strupr(cadena), "PRIMER")==0)
		enumP=PRIMER_AJUSTE;
	else
		if (strcmp(strupr(cadena), "MEJOR")==0)
			enumP=MEJOR_AJUSTE;
		else
			if (strcmp(strupr(cadena), "PEOR")==0)
				enumP=PEOR_AJUSTE;

	return (enumP);
}


/* Devuelve el nombre de un gestor de memoria */
char *NombreGestorMemoria (enum GestorMemoria gestMem, char *buffer)
{
	switch(gestMem)
	{
		case GESNINGUNO:
			strcpy(buffer, "Ninguno");
			break;
		case PARTICION_ESTATICA:
			strcpy(buffer, "Particiขn est tica");
			break;
		case PARTICION_DINAMICA:
			strcpy(buffer, "Particiขn din mica");
			break;
		case PAGINACION:
			strcpy(buffer, "Paginaciขn");
			break;
	}

	return (buffer);
}


/* Devuelve el enumerado de un gestor de memoria */
enum GestorMemoria EnumGestorMemoria(char *cadena)
{
	static enum GestorMemoria enumG;

	if (strcmp(strupr(cadena), "NINGUNO")==0)
		enumG=GESNINGUNO;
	else
		if (strcmp(strupr(cadena), "ESTATICA")==0)
			enumG=PARTICION_ESTATICA;
		else
			if (strcmp(strupr(cadena), "DINAMICA")==0)
				enumG=PARTICION_DINAMICA;
			else
				if (strcmp(strupr(cadena), "PAGINADO")==0)
					enumG=PAGINACION;

	return (enumG);
}


/* Devuelve el mayor bloque de memoria que puede ocupar un proceso */
int MemoriaMaxima(void *mem, enum GestorMemoria gestMem)
{
	static int memT;

	switch (gestMem)
	{
		case GESNINGUNO:
			memT=((struct MemNinguno *)mem)->memTotal;
			break;

		case PARTICION_ESTATICA:
			memT=((struct MemParticiones *)mem)->memTotal/2;
			break;
		case PARTICION_DINAMICA:
			memT=((struct MemParticiones *)mem)->memTotal;
			break;

		case PAGINACION:
			memT=((struct MemPaginas *)mem)->memTotal;
			break;
	}
	return (memT);
}


/* Devuelve el mayor bloque de memoria que hay disponible */
int MemoriaDisponible(void *mem, enum GestorMemoria gestMem)
{
	static int memDisp;

	struct ListaParticiones *busqSeg;

	switch (gestMem)
	{
		case GESNINGUNO:
			memDisp=((struct MemNinguno *)mem)->memLibre;
			break;

		case PARTICION_ESTATICA:
		case PARTICION_DINAMICA:
			memDisp=0;
			busqSeg=((struct MemParticiones *)mem)->mapa;

			while (busqSeg!=NULL)
			{
				if (busqSeg->est==LIBRE)
					memDisp=MAYOR(memDisp, busqSeg->longitud);

				busqSeg=busqSeg->sig;
			}
			break;

		case PAGINACION:

			memDisp=((struct MemPaginas *)mem)->tamPagina*((struct MemPaginas *)mem)->pagLibres;
			break;
	}

	return (memDisp);
}


/* Devuelve un vector con el estado de la memoria */
enum EstadoMemoria *MapaMemoria(void *mem, enum GestorMemoria gestMem)
{
	static enum EstadoMemoria mapa[DIVMEMORIA];
	struct ListaParticiones *busqSeg=((struct MemParticiones *)mem)->mapa;
	double cuadro, cont2=0.0;
	int cont;

	switch (gestMem)
	{
		case GESNINGUNO:
			cuadro=((struct MemNinguno *)mem)->memTotal/DIVMEMORIA;

			for (cont=0; cont<DIVMEMORIA; cont++)
				if (cont*cuadro < ((struct MemNinguno *)mem)->memTotal-((struct MemNinguno *)mem)->memLibre)
					mapa[cont]=ASIGNADA;
				else
					mapa[cont]=LIBRE;

			break;

		case PARTICION_ESTATICA:
		case PARTICION_DINAMICA:
			cuadro=((struct MemParticiones *)mem)->memTotal/DIVMEMORIA;

			for (cont=0; cont<DIVMEMORIA; cont++)
			{
				mapa[cont]=busqSeg->est;
				cont2+=cuadro;

				if (cont2>=busqSeg->longitud)
				{
					cont2-=(double)busqSeg->longitud;
					busqSeg=busqSeg->sig;
				}
			}
			break;

		case PAGINACION:

			for (cont=0; cont<DIVMEMORIA; cont++)
				mapa[cont]=((struct MemPaginas *)mem)->mapa[cont].est;

			break;
	}

	return (mapa);
}


#endif