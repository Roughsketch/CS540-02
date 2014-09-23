CC=gcc
CFLAGS=-c -std=c99 -Wall -pthread
LDFLAGS=
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

ifeq ($(OS), Windows_NT)
	EXECUTABLE=sudoku.exe
	CLEAN=del /f
else
	EXECUTABLE=sudoku
	CLEAN=rm -rf
endif


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(CLEAN) $(OBJECTS)
	$(CLEAN) $(EXECUTABLE)

cleanobj:
	$(CLEAN) $(OBJECTS)

rebuild: clean $(EXECUTABLE) cleanobj
