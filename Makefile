CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -ljson-c
TARGET = bank.cgi
SRC = bank.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c bank.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) *.dat

install: $(TARGET)
	cp $(TARGET) /usr/lib/cgi-bin/
	chmod +x /usr/lib/cgi-bin/$(TARGET)

.PHONY: all clean install
