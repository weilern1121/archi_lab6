all: f

f:  line_parser.o main.o
	gcc -g -Wall -o f line_parser.o main.o

line_parser.o: line_parser.c
	gcc -g -Wall -c -o line_parser.o line_parser.c
	
main.o: main.c
	gcc -g -Wall -c -o main.o main.c

clean:
	rm -f *.o
 
