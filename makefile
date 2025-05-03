CC = gcc
CFLAGS = -Wall -D_XOPEN_SOURCE_EXTENDED=1
LDFLAGS = -lncursesw
TARGET = main
SRC = main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
