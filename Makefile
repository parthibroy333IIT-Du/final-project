CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -liup

TARGET = image_editor
SRCS = main.c image.c image_processing.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
