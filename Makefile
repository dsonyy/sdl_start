TARGET = game
LIBS = -lm -lSDL2 -lSDL2_image
CC = g++
CFLAGS = -O3 -Wall -pedantic -Wextra -std=c++20

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)