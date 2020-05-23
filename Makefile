main : file.o main.o
	gcc file.o main.o -lpthread -o main 

file.o : file.c file.h
	gcc -o file.o -c file.c

main.o : main.c file.h
	gcc -lpthread -o main.o -c main.c

clear : 
	rm *.o
	rm main
	rm tube.fifo