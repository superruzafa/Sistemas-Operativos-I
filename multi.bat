sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p FCFS -g ESTATICA -a PRIMER
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p SJF -g ESTATICA -a PRIMER
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p SRT -g ESTATICA -a PRIMER
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p RR -g ESTATICA -q 6 -a PRIMER

sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p FCFS -g DINAMICA -a MEJOR
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p SJF -g DINAMICA -a MEJOR
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p SRT -g DINAMICA -a MEJOR
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p RR -g DINAMICA -q 6 -a MEJOR

sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p FCFS -g PAGINADO
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p SJF -g PAGINADO
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p SRT -g PAGINADO
sch -e carga98.sch -l sucesos.txt -s MULTISINP -m 512 -c 10 -p RR -g PAGINADO -q 6


