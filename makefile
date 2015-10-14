LabAss2 : LabAss2.o
	g++ LabAss2.o -o LabAss2

LabAss2.o : LabAss2.cpp
	g++ -c LabAss2.cpp

clean :
	rm *.o LabAss2
