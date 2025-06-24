CC = gcc
FLAGS = -Wall -Wextra
SOURCES = main.c command.c string_parser.c 
OBJECTS = $(SOURCES:.c=.o)
TARGET = pseudo-shell

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

all: $(TARGET)
