CC = gcc
CFLAGS = -Iinclude -Wall -O2
TARGET = logkey_client

SRCS = src/main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
