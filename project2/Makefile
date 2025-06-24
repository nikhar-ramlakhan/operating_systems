CC = gcc -g

all: part1 part2 part3 part4 part5 cpubound iobound

part1: part1.o string_parser.o
	$(CC) -o $@ $^

part2: part2.o string_parser.o
	$(CC) -o $@ $^

part3: part3.o string_parser.o
	$(CC) -o $@ $^

part4: part4.o string_parser.o
	$(CC) -o $@ $^

part5: part5.o string_parser.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c $<

cpubound: cpubound.c
	$(CC) $< -o $@

iobound: iobound.c
	$(CC) $< -o $@

string_parser.o: string_parser.c string_parser.h
	$(CC) -c $<

clean:
	rm -f core *.o part1 part2 part3 part4 part5 cpubound iobound
