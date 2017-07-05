/*ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
/*บ Planificador de procesos                                                 บ*/
/*บ                                                                          บ*/
/*บ Sistemas operativos I                                                    บ*/
/*บ                                                                          บ*/
/*บ Alfonso Ruzafa Molina                                                    บ*/
/*บ Alejandro Sola V zquez                                                   บ*/
/*บ 3ง A ITI Sistemas                                                        บ*/
/*บ                                                                          บ*/
/*บ Fichero: Interfaz.h                                                      บ*/
/*บ          Funciones para la entrada de datos y la representaciขn de la    บ*/
/*บ          simulaciขn                                                      บ*/
/*ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
#if !defined(INTERFAZ_DEF)
#define INTERFAZ_DEF

#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
#include "colas.h"
#include "memoria.h"

#define BVIDEO 0xB8000000

#define FILAS 25
#define COLUMNAS 80

#define DIRVIDEO(b, x, y) (char far *)(b + x*2 + COLUMNAS*y*2)

#define COLORFONDO 1

#define COLORPROGRESO 12
#define MEMORIALIBRE 6
#define MEMORIAASIGNADA 14

#define ESCAPE 27
#define TAB 9
#define BS 8
#define RETORNO 13
#define INICIO 71
#define FIN 79
#define REPAG 73
#define AVPAG 81
#define ARRIBA 72
#define ABAJO 80
#define IZQ 75
#define DCH 77

/*----------------------------------------------------------------------------
Prototipos de funciones */

void EscribirCaracter(char, int, int, int, int);
void EscribirTexto(char *, int, int y, int, int);
void EscribirVacio(int, int, int, int);
void DibujarCuadro(int, int, int, int);
int ColorProceso(enum Prioridad);

void DibujarPCBs (struct PCB *, int, int);
void DibujarInterfaz(char **);
void DibujarMemoria(enum EstadoMemoria *);
void DibujarProgreso(int);
void EscribirProcEjec(int);
void EscribirCCTX(int);
void EscribirEvento(struct PCB *, char *);
void EscribirReloj(unsigned long);
void EscribirPCBs(int);

int EntrarDatos(char *, char *, enum ModoSimulacion *, enum Politica *, unsigned int *, enum GestorMemoria *, enum AjusteParticion *, unsigned int *, unsigned int *, unsigned int *);
int EscogerOpcion(int, int, char **, int *, int, int, int);
int LeerTexto(char *, int, int, int);
int LeerNumero(unsigned int *, int, int, int, int, int);


/*----------------------------------------------------------------------------
Cuerpo de funciones */

/* Escribe un caracter en una posiciขn de la pantalla */
void EscribirCaracter(char caracter, int x, int y, int colorT, int colorF)
{
	char far *ptr;
	ptr=DIRVIDEO(BVIDEO, x, y);

	*ptr++=caracter;

	/* Para quitar el bit de parpadeo */
	*ptr=*ptr & 0xEF;

	/* Color de la letra */
	if (colorT!=-1)
		*ptr=(*ptr & 0xE0) + (char)colorT;

	/* Color de fondo */
	if (colorF!=-1)
		*ptr=(*ptr & 0x0F) + ((char)colorF<<4);
}


/* Escribe una cadena de caracteres en una posiciขn de la pantalla */
void EscribirTexto(char *texto, int x, int y, int colorT, int colorF)
{
	int cont=0;
	int xInicio=x;
	char bufColorTexto[5], bufColorFondo[5];

	while (texto[cont]!='\0')
	{
		switch(texto[cont])
		{
			/* Retorno */
			case '\n':
				x=xInicio;
				y++;
				break;

			/* Caracter de color de texto */
			case '$':
				strset(bufColorTexto, '\0');
				colorT=atoi(strncpy(bufColorTexto, &texto[cont+1], 2));
				cont+=2;
				break;

			/* Caracter de color de fondo */
			case '&':
				strset(bufColorFondo, '\0');
				colorF=atoi(strncpy(bufColorFondo, &texto[cont+1], 2));
				cont+=2;
				break;

			/* Otro caracter */
			default:
				EscribirCaracter(texto[cont], x++, y, colorT, colorF);
				break;
		}
		cont++;
	}
}


/* Escribe un rect ngulo de espacios en blanco en la pantalla */
void EscribirVacio(int x, int y, int ancho, int alto)
{
	int x2, y2;

	for (x2=0; x2<ancho; x2++)
		for(y2=0; y2<alto; y2++)
			EscribirTexto(" ", x+x2, y+y2, -1, COLORFONDO);

}


/* Dibuja un cuadro de un color con un nฃmero dentro */
void DibujarCuadro(int x, int y, int colorM, int nPro)
{
	char buffer[10];

	itoa(nPro, buffer, 10);

	EscribirTexto("ฺฤฤฟ\nณ  ณ\nภฤฤู", x, y, colorM, COLORFONDO);

	if (nPro<10)
	{
		EscribirTexto("0", x+1, y+1, 15, colorM-8);
		EscribirTexto(buffer, x+2,y+1, 15, colorM-8);
	}
	else
		EscribirTexto(buffer, x+1, y+1, 15, colorM-8);
}


/* Dibuja una serie de cuadros con un nฃmero dentro */
void DibujarPCBs (struct PCB *inicio, int x, int y)
{
	struct PCB *pcb=inicio;

	while (pcb!=NULL && x<COLUMNAS-4)
	{
		DibujarCuadro(x, y, ColorProceso(pcb->prio), pcb->numProceso);
		x+=4;
		pcb=pcb->sig;
	}

	/* Se han quedado PCBs sin dibujar */
	if (pcb!=NULL)
		EscribirTexto("->", x, y+1, 15, COLORFONDO);
}


/* Devuelve el color asociado a cada prioridad */
int ColorProceso(enum Prioridad p)
{
	static int color;

	switch (p)
	{
		case TR:
			color=13;
			break;

		case SYS:
			color=12;
			break;

		case BAT:
			color=10;
			break;

		default:
			color=14;
			break;
	}

	return(color);
}


/* Dibuja la interfaz con la configuraciขn de la simulacion */
void DibujarInterfaz(char **param)
{
	clrscr();

	EscribirTexto("\
ษ $15Estado del sistema$07 ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\
บ ษออออป      ษออออป      ษออออป                                               บ\
บ บ$15CPU $07ฬ>     บ$15DISP$07ฬ>     บ$15SWAP$07ฬ>                                              บ\
บ ศออออผ      ศออออผ      ศออออผ                                               บ\
บ ษออออป                                                                       บ\
บ บ$15LIST$07ฬ>                                                                      บ\
บ ศออออผ                                                                       บ\
บ ษออออป                                                                       บ\
บ บ$15BLOQ$07ฬ>                                                                      บ\
บ ศออออผ                                                                       บ\
ฬ $15Memoria$07 อออออออออออออออออออออออห $15Evento$07 อออออออออออออออออออออออออออออออออออออน\
บ                                บ                                             บ\
บ                                ฬ $15Reloj$07 ออออห $15PCBs$07 ห $15Camb CTX.$07 ห $15Proc. ejec$07 ออน\
บ                                บ           บ      บ           บ              บ\
บ                                ฬ $15Progreso$07 อสออออออสอออออออออออสออออออออออออออน\
บ þ =                            บ                                             บ\
ฬ $15Par metros$07 ออออออออออออออออออออสอออออออออออออออออออออออออออออออออออออออออออออน\
บ Entrada.......:                             Salida....:                      บ\
บ Simulaciขn....:                             Memoria...:                      บ\
บ Polกtica......:                             Quantum...:                      บ\
บ Gestor memoria:                             Ajuste....:                      บ\
บ TIO...........:                             PCBs......:                      บ\
ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\
บ $15ESC$07: Salir                  $15<ู$07: Avance r pido            $15Otra tecla$07: Avanzar บ\
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ", 0, 0, 7, COLORFONDO);

	EscribirTexto(param[0], 18, 17, 11, COLORFONDO);
	EscribirTexto(param[2], 18, 18, 11, COLORFONDO);
	EscribirTexto(param[3], 18, 19, 11, COLORFONDO);
	EscribirTexto(param[4], 18, 20, 11, COLORFONDO);
	EscribirTexto(param[5], 18, 21, 11, COLORFONDO);

	EscribirTexto(param[1], 58, 17, 11, COLORFONDO);
	EscribirTexto(param[6], 58, 18, 11, COLORFONDO);
	EscribirTexto(param[7], 58, 19, 11, COLORFONDO);
	EscribirTexto(param[8], 58, 20, 11, COLORFONDO);
	EscribirTexto(param[9], 58, 21, 11, COLORFONDO);

	EscribirTexto("þ =", 2, 15, 11, COLORFONDO);
	EscribirTexto(param[10], 6, 15, 11, COLORFONDO);
}


/* Dibuja el estado de la memoria */
void DibujarMemoria(enum EstadoMemoria *mapa)
{
	int x=1, y=11, cont;

	for (cont=0; cont<DIVMEMORIA; cont++)
	{
		if (mapa[cont]==LIBRE)
			EscribirTexto("þ", x++, y, MEMORIALIBRE, COLORFONDO);
		else
			EscribirTexto("þ", x++, y, MEMORIAASIGNADA, COLORFONDO);

		if (x==33)
		{
			x=1; y++;
		}
	}
}


/* Dibuja el progreso de la simulacion */
void DibujarProgreso(int porc)
{
	char buffer[4];
	int x=34;
	double cuadro=0.0;

	EscribirVacio(34, 15, 45, 1);

	while (x<75 && cuadro<porc)
	{
		EscribirTexto("þ", x++, 15, COLORPROGRESO, COLORFONDO);
		cuadro+=(double)100/41;
	}


	EscribirTexto(strcat(itoa(porc, buffer, 10), "%"), x, 15, COLORPROGRESO, COLORPROGRESO-8);

}


/* Escribe el nฃmero de procesos que se est n ejecutando */
void EscribirProcEjec(int procEjec)
{
	char buffer[6];

	EscribirVacio(66, 13, 6, 1);
	EscribirTexto(itoa(procEjec, buffer, 10), 66, 13, 10, COLORFONDO);
}

/* Escribe el nฃmero de cambios de contexto ocurridos */
void EscribirCCTX(int cambios)
{
	char buffer[6];

	EscribirVacio(54, 13, 6, 1);
	EscribirTexto(itoa(cambios, buffer, 10), 54, 13, 10, COLORFONDO);
}


/* Escribe un mensaje diciendo lo que le ha pasado a un proceso */
void EscribirEvento(struct PCB *proceso, char *texto)
{
	char id[4];
	char *ptr=texto;
	int x=35;

	EscribirVacio(35, 11, 44, 1);

	while (*ptr!='\0' && *ptr!='_')
		EscribirCaracter(*ptr++, x++, 11, 10, COLORFONDO);

	if (*ptr=='_' && proceso!=NULL)
	{
		if (proceso->numProceso<10)
			EscribirCaracter('0', x++, 11, 15, ColorProceso(proceso->prio)-8);

		EscribirTexto(itoa(proceso->numProceso, id, 10), x, 11, 15, ColorProceso(proceso->prio)-8);
		x+=strlen(id);
		ptr++;
	}

	EscribirTexto(ptr, x, 11, 10, COLORFONDO);
}


/* Escribe el tiempo de simulacion */
void EscribirReloj(unsigned long reloj)
{
	char buffer[10];

	EscribirVacio(35, 13, 6, 1);
	EscribirTexto(ultoa(reloj, buffer, 10), 35, 13, 10, COLORFONDO);
}


/* Escribe el nฃmero de PCBs disponibles del sistema */
void EscribirPCBs(int nPCB)
{
	char buffer[10];

	EscribirVacio(47, 13, 5, 1);
	EscribirTexto(itoa(nPCB, buffer, 10), 47, 13, 10, COLORFONDO);
}

/* Presenta el interfaz para configurar la simulacion */
int EntrarDatos(char *entrada, char *salida, enum ModoSimulacion *simulacion, enum Politica *politica, unsigned int *quantum, enum GestorMemoria *gestMem, enum AjusteParticion *ajuste, unsigned int *memoria, unsigned int *tio, unsigned int *pcbs)
{
	#define TECLACONTROL(x) (x==TAB || x==RETORNO || x==ESCAPE)

	enum Datos
	{
		ENTRADA, SIMULACION, POLITICA, GESTOR, TIO, SALIDA, MEMORIA, QUANTUM, AJUSTE, PCBS
	} opActual=ENTRADA;

	int tecla;
	int cont;
	char buffer[6];

	char opSimulacion[4][30];
	char *ptrSimulacion[4];

	char opPolitica[4][30];
	char *ptrPolitica[4];

	char opGestor[4][30];
	char *ptrGestor[4];

	char opAjuste[4][30];
	char *ptrAjuste[4];

	for (cont=0; cont<4; cont++)
	{
		NombreSimulacion(cont, opSimulacion[cont]);
		ptrSimulacion[cont]=opSimulacion[cont];

		NombrePolitica(cont, opPolitica[cont]);
		ptrPolitica  [cont]=opPolitica[cont];

		NombreGestorMemoria(cont, opGestor[cont]);
		ptrGestor    [cont]=opGestor[cont];

		if (cont<3)
			NombreAjuste(cont+1, opAjuste[cont+1]);
		ptrAjuste[cont]=opAjuste[cont];
	}
	strcpy(opAjuste[0], "Desactivado");

EscribirTexto("\
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\
บ                                                                              บ\
บ  þ $10Planificador de Sistema Operativo$07                                         บ\
บ  þ $10Sistemas Operativos I$07                                                     บ\
บ  þ $10Ingenierกa Tcnica en Inform tica de Sistemas$07                             บ\
บ  þ $10Curso 2000/2001$07                                                           บ\
บ                                                                              บ\
บ  þ $11Alfonso Ruzafa Molina$07                                                     บ\
บ  þ $11Alejandro Sola V zquez$07                                                    บ\
บ  þ $113ง A$07                                                                      บ\
บ                                                                              บ\
บ                                                                              บ\
บ                                                                              บ\
บ                                                                              บ\
บ                                                                              บ\
บ                                                                              บ\
ฬ $15Par metros$07 ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\
บ Entrada.......:                             Salida....:                      บ\
บ Simulaciขn....:                             Memoria...:                      บ\
บ Polกtica......:                             Quantum...:                      บ\
บ Gestor memoria:                             Ajuste....:                      บ\
บ TIO...........:                             PCBs......:                      บ\
ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\
บ $15TAB$07: Siguente op.   $15RETURN$07: Aceptar   $15ESC$07: Salir   $15Flechas$07: Cambiar opciones บ\
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ", 0, 0, 7, COLORFONDO);

	do
	{
		/* Se asegura de que la configuraciขn actual es correcta */
		if (MONOPROCESO(*simulacion))
		{
			*politica=FCFS;
			*quantum=0;
			*gestMem=GESNINGUNO;
			*ajuste=AJNINGUNO;

			if (*simulacion==MONOPRO_SINES)
				*tio=0;
			else
				*tio=MAYOR(*tio, 1);
		}
		else
		{
			*gestMem=MAYOR(*gestMem, PARTICION_ESTATICA);
			if (MEMORIAPARTICIONADA(*gestMem))
			{
				*ajuste=MAYOR(*ajuste, PRIMER_AJUSTE);

				if (*gestMem==PARTICION_ESTATICA)
					*ajuste=MENOR(*ajuste, MEJOR_AJUSTE);
			}
			else
				*ajuste=AJNINGUNO;

			*tio=MAYOR(*tio, 1);
			if (*politica==RR)
				*quantum=MAYOR(*quantum, 1);
		}


		/* Escribe la configuraciขn actual */

		EscribirVacio(18, 17, 27, 5);
		EscribirVacio(58, 17, 20, 5);

		EscribirTexto(entrada,                   18, 17, 10, COLORFONDO);
		EscribirTexto(opSimulacion[*simulacion], 18, 18, 10, COLORFONDO);
		EscribirTexto(opPolitica[*politica],     18, 19, 10, COLORFONDO);
		EscribirTexto(opGestor[*gestMem],        18, 20, 10, COLORFONDO);
		EscribirTexto(itoa(*tio, buffer, 10),    18, 21, 10, COLORFONDO);

		EscribirTexto(salida,                     58, 17, 10, COLORFONDO);
		EscribirTexto(itoa(*memoria, buffer, 10), 58, 18, 10, COLORFONDO);
		EscribirTexto(itoa(*quantum, buffer, 10), 58, 19, 10, COLORFONDO);
		EscribirTexto(opAjuste[*ajuste],          58, 20, 10, COLORFONDO);
		EscribirTexto(itoa(*pcbs, buffer, 10),    58, 21, 10, COLORFONDO);

		/* Ofrece la posibilidad de modificar el campo actual y desplazarse
		al siguiente */

		switch(opActual)
		{
			case ENTRADA:
				tecla=LeerTexto(entrada, 18, 17, 27);
				opActual++;
				break;

			case SALIDA:
				tecla=LeerTexto(salida, 58, 17, 20);
				opActual++;
				break;

			case SIMULACION:
				tecla=EscogerOpcion(0, 3, ptrSimulacion, (int *)simulacion, 18, 18, 27);

				if (TECLACONTROL(tecla))
					if MULTIPROCESO(*simulacion)
						opActual++;
					else
						if (*simulacion==MONOPRO_CONES)
							opActual=TIO;
						else
							opActual=SALIDA;

				break;

			case POLITICA:
				tecla=EscogerOpcion(0, 3, ptrPolitica, (int *)politica, 18, 19, 27);

				if (TECLACONTROL(tecla))
					opActual++;

				break;

			case QUANTUM:
				tecla=LeerNumero(quantum, MAXINTEGER, 1, 58, 19, 20);

				if (TECLACONTROL(tecla))
					if (MEMORIAPARTICIONADA(*gestMem))
						opActual++;
					else
						opActual=PCBS;

				break;

			case GESTOR:
				tecla=EscogerOpcion(1, 3, ptrGestor, (int *)gestMem, 18, 20, 27);

				if (TECLACONTROL(tecla))
					opActual++;

				break;

			case AJUSTE:
				tecla=EscogerOpcion(1, 3, ptrAjuste, (int *)ajuste, 58, 20, 20);

				if (TECLACONTROL(tecla))
					opActual++;

				break;

			case MEMORIA:
				tecla=LeerNumero(memoria, MAXINTEGER, 1, 58, 18, 20);

				if (TECLACONTROL(tecla))
					if (MULTIPROCESO(*simulacion))
						if (*politica==RR)
							opActual=QUANTUM;
						else
							if (MEMORIAPARTICIONADA(*gestMem))
								opActual=AJUSTE;
							else
								opActual=PCBS;
					else
						opActual=PCBS;

				break;

			case TIO:
				tecla=LeerNumero(tio, MAXINTEGER, 1, 18, 21, 27);

				if (TECLACONTROL(tecla))
					opActual++;

				break;

			case PCBS:
				tecla=LeerNumero(pcbs, MAXINTEGER, 1, 58, 21, 20);

				if (TECLACONTROL(tecla))
					opActual=ENTRADA;

				break;
		}
	}
	while (tecla!=ESCAPE && tecla!=RETORNO);

	return (tecla);
}


/* Escoge una opciขn entre las que se le pasan */
int EscogerOpcion(int min, int max, char *opciones[], int *valor, int x, int y, int longi)
{
	int tecla=0;

	EscribirVacio(x, y, longi, 1);
	EscribirTexto(opciones[*valor], x, y, 1, 3);

	tecla=getch();
	if (tecla==0)
		tecla=getch();

	switch(tecla)
	{
		case IZQ:
		case ARRIBA:
		case REPAG:
			*valor=MAYOR(*valor-1, min);
			break;

		case DCH:
		case ABAJO:
		case AVPAG:
			*valor=MENOR(*valor+1, max);
			break;

		case INICIO:
			*valor=min;
			break;

		case FIN:
			*valor=max;
			break;
	}

	return(tecla);
}


/* Lee un texto y lo guarda en variable */
int LeerTexto(char *texto, int x, int y, int longi)
{
	int pos=strlen(texto);
	int tecla;

	do
	{
		EscribirVacio(x, y, longi, 1);
		EscribirTexto(texto, x, y, 1, 3);
		gotoxy(x+strlen(texto)+1, y+1);

		tecla=getch();

		if (isalnum(tecla) || ispunct(tecla))
			texto[pos++]=(char)tecla;
		else
			if (tecla==BS)
				texto[pos=MAYOR(0, pos-1)]='\0';

		if (tecla==0)
			getch();
	}
	while (!(tecla==ESCAPE || tecla==RETORNO || tecla==TAB));

	gotoxy(1, 1);

	return(tecla);
}


/* Lee un nฃmero y lo guarda en una variable */
int LeerNumero(unsigned int *valor, int max, int min, int x, int y, int longi)
{
	char numC[10];
	int tecla=0;
	int entrado=0;

	do
	{
		EscribirVacio(x, y, longi, 1);
		EscribirTexto(itoa(*valor, numC, 10), x, y, 1, 3);
		gotoxy(x+strlen(itoa(*valor, numC, 10))+1, y+1);
		tecla=getch();
		if (tecla==0)
			tecla=getch();

		switch(tecla)
		{
			case REPAG:
				*valor=MAYOR(*valor-10, min);
				break;


			case IZQ:
			case ABAJO:
				*valor=MAYOR(*valor-1, min);
				break;

			case AVPAG:
				*valor=MENOR(*valor+10, max);
				break;

			case DCH:
			case ARRIBA:
				*valor=MENOR(*valor+1, max);
				break;

			case INICIO:
				*valor=min;
				break;

			case FIN:
				*valor=max;
				break;

			case 48: case 49: case 50: case 51: case 52:
			case 53: case 54: case 55: case 56: case 57:
				if (entrado)
					*valor=(*valor*10)+(char)tecla-48;
				else
					*valor=(char)tecla-48;

				break;

			case BS:
				*valor/=10;
				break;
		}
		entrado=1;
	}
	while (!(tecla==ESCAPE || tecla==RETORNO || tecla==TAB));

	gotoxy(1, 1);

	return(tecla);
}



#endif
