CC = gcc

all: bank

bank: bank.o string_parser.o
	$(CC) -pthread -lpthread $^ -o $@

bank.o: bank.c
	$(CC) -c $<

string_parser.o: string_parser.c string_parser.h
	$(CC) -c $<

clean:
	rm -f core *.o bank
