/*ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
/*บ Planificador de procesos                                                 บ*/
/*บ                                                                          บ*/
/*บ Sistemas operativos I                                                    บ*/
/*บ                                                                          บ*/
/*บ Alfonso Ruzafa Molina                                                    บ*/
/*บ Alejandro Sola V zquez                                                   บ*/
/*บ 3ง A ITI Sistemas                                                        บ*/
/*บ                                                                          บ*/
/*บ Fichero: Util.h                                                          บ*/
/*บ          Funciones diversas                                              บ*/
/*บ                                                                          บ*/
/*ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
#if !defined(UTIL_DEF)
#define UTIL_DEF

#define MAXINTEGER 32767

#define MAYOR(x, y) (x > y ? x : y)
#define MENOR(x, y) (x < y ? x : y)

#define MONOPROCESO(x)  ((x==MONOPRO_SINES || x==MONOPRO_CONES) ? 1 : 0)
#define MULTIPROCESO(x) ((x==MULTIPRO_SINPRIO || x==MULTIPRO_CONPRIO) ? 1 : 0)

#define MAXLINEA 80

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "colas.h"

/* Enumerado con los tipos de simulaciones */
enum ModoSimulacion
{
	MONOPRO_SINES, MONOPRO_CONES, MULTIPRO_SINPRIO, MULTIPRO_CONPRIO
};

/* Enumerado con las politicas de planificaciขn */
enum Politica
{
	FCFS, SJF, SRT, RR
};

/*---------------------------------------------------------------------------
Prototipos de funciones */

struct Cola        *AbrirSimulacion(char *);
struct PCB         *Desmembrar(char *);
char               *PPalabra(char **);
enum Prioridad      Prioridad(char *);
int                 TotalComputo(struct Cola *);
char               *NombreSimulacion(enum ModoSimulacion, char *);
enum ModoSimulacion EnumSimulacion(char *);
char               *NombrePolitica(enum Politica, char *);
enum Politica       EnumPolitica(char *);
char               *Espacios(char *, int );


/*---------------------------------------------------------------------------
Cuerpo de funciones */

/* Devuelve una cola de carga a partir de un fichero de carga */
struct Cola *AbrirSimulacion(char *archivo)
{
	FILE *fSimul;
	static struct Cola *carga=NULL;
	struct PCB *proceso=NULL;

	char linea[MAXLINEA];

	carga=CrearCola();

	if ((fSimul=fopen(archivo, "rt"))!=NULL)
	{
		fgets(linea, MAXLINEA, fSimul);
		fgets(linea, MAXLINEA, fSimul);

		while (!feof(fSimul))
		{
			strcpy(linea, "");
			fgets(linea, MAXLINEA, fSimul);

			proceso=Desmembrar(linea);

			if (proceso!=NULL)
				MeteCola(carga, proceso, OrdenHLLeg);
		}
		fclose(fSimul);
	}
	return (carga);
}

/* Devuelve un PCB con la informaciขn contenida en una linea */
struct PCB *Desmembrar(char *linea)
{
	char palabra[6][10];
	int cont, valida=1;

	for (cont=0; cont<6 && valida; cont++)
	{
		strcpy(palabra[cont], PPalabra(&linea));
		valida=strcmp(palabra[cont], "")!=0;
	}

	if (valida)
		return (CrearPCB(atoi	   (palabra[0]),
						 atoi	   (palabra[1]),
						 atoi	   (palabra[2]),
						 atoi      (palabra[3]),
						 atoi      (palabra[4]),
						 Prioridad (palabra[5]) ));
	else
		return(NULL);
}


/* Devuelve la primera palabra contenida en una cadena modific ndola */
char *PPalabra(char **linea)
{
	static char palabra[10];
	int cont=0;

	while (!isalnum(**linea) && **linea!='\0')
		++*linea;

	while (isalnum(**linea) && **linea!='\0')
	{
		palabra[cont]=**linea;
		cont++;
		++*linea;
	}
	palabra[cont]='\0';

	return (palabra);
}

/* Devuelve el enumerado de una prioridad */
enum Prioridad Prioridad(char *cadena)
{

	if (strcmp(cadena, "TR")==0)
		return (TR);
	if (strcmp(cadena, "SYS")==0)
		return (SYS);
	if (strcmp(cadena, "INT")==0)
		return (INT);

	return (BAT);
}

/* Devuelve la suma del computo de los procesos de una cola */
int TotalComputo(struct Cola *cola)
{
	struct PCB *proceso=cola->inicio;
	static int totalC;

	totalC=0;

	while (proceso!=NULL)
	{
		totalC+=proceso->totalCPU;
		proceso=proceso->sig;
	}

	return (totalC);
}


/* Devuelve el nombre de una simulacion */
char *NombreSimulacion(enum ModoSimulacion m, char *buffer)
{
	switch (m)
	{
		case MONOPRO_SINES:
			strcpy(buffer, "Monoproceso sin E/S");
			break;

		case MONOPRO_CONES:
			strcpy(buffer, "Monoproceso con E/S");
			break;

		case MULTIPRO_SINPRIO:
			strcpy(buffer, "Multiproceso sin prioridad");
			break;

		case MULTIPRO_CONPRIO:
			strcpy(buffer, "Multiproceso con prioridad");
			break;
	}
	return(buffer);
}

/* Devuelve el enumerado de una simulaciขn */
enum ModoSimulacion EnumSimulacion(char *cadena)
{
	static enum ModoSimulacion enumS;

	if (strcmp(strupr(cadena), "MONOSINES")==0)
		enumS=MONOPRO_SINES;
	else
		if (strcmp(strupr(cadena), "MONOCONES")==0)
			enumS=MONOPRO_CONES;
		else
			if (strcmp(strupr(cadena), "MULTISINP")==0)
				enumS=MULTIPRO_SINPRIO;
			else
				if (strcmp(strupr(cadena), "MULTICONP")==0)
					enumS=MULTIPRO_CONPRIO;

	return (enumS);
}


/* Devuelve el nombre de una polกtica */
char *NombrePolitica(enum Politica p, char *buffer)
{
	switch (p)
	{
		case FCFS:
			strcpy(buffer, "First Comed First Served");
			break;

		case SJF:
			strcpy(buffer, "Shortest Job First");
			break;

		case SRT:
			strcpy(buffer, "Shortest Remainder Time");
			break;

		case RR:
			strcpy(buffer, "Round Robin");
			break;
	}
	return (buffer);
}


/* Devuelve el enumerado de una politica */
enum Politica EnumPolitica(char *cadena)
{
	static enum Politica enumP;

	if (strcmp(strupr(cadena), "FCFS")==0)
		enumP=FCFS;
	else
		if (strcmp(strupr(cadena), "SJF")==0)
			enumP=SJF;
		else
			if (strcmp(strupr(cadena), "SRT")==0)
				enumP=SRT;
			else
				if (strcmp(strupr(cadena), "RR")==0)
					enumP=RR;
	return(enumP);
}

/* Rellena una cadena con espacios */
char *Espacios(char *buffer, int num)
{
	strcpy(buffer, "");

	while (num>0)
	{
		strcat(buffer, " ");
		num--;
	}

	return (buffer);
}




char *dtoa(double numero, char *buffer, int decimales)
{
	int cont;

	itoa((int)numero, buffer, 10);

	strcat(buffer, ",");
	numero-=(int)numero;
	cont=strlen(buffer);

	do
	{
		buffer[cont++]=(int)(numero*=10)+48;
		numero-=(int)numero;
	}
	while (numero>0 && --decimales>0);

	buffer[cont]='\0';

	return (buffer);
}








#endif