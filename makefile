LabAss2 : LabAss2.o
	gcc LabAss2.o -o LabAss2

LabAss2.o : LabAss2.c
	gcc -c LabAss2.c

clean :
	rm *.o LabAss2
