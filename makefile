all: text_analysis

text_analysis: text_analysis.o 
	mpicc -std=c99 -Wall -pedantic text_analysis.o -o text_analysis

text_analysis.o: text_analysis.c
	mpicc -std=c99 -Wall -pedantic -c text_analysis.c -o text_analysis.o

clean:
	rm -f *.o text_analysis
