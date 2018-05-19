all: f

f:  line_parser.o main.o job_control.o
	gcc -g -Wall -o f line_parser.o main.o job_control.o

line_parser.o: line_parser.c
	gcc -g -Wall -c -o line_parser.o line_parser.c
	
main.o: main.c
	gcc -g -Wall -c -o main.o main.c

job_control.o: job_control.c
	gcc -g -Wall -c -o job_control.o job_control.c

clean:
	rm -f *.o
 
